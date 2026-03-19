// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Enemies/EnemyBase.h"
#include "Grid/BombermanGrid.h"
#include "Core/BombermanGameMode.h"
#include "Player/BombermanCharacter.h"
#include "Components/BombermanHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UBombermanHealthComponent>(TEXT("HealthComponent"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	bUseControllerRotationYaw = false;

	GetCapsuleComponent()->SetCapsuleSize(30.f, 60.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	// --- debug shiit ---
	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	DirectionArrow->SetupAttachment(RootComponent);

#if WITH_EDITOR
	DirectionArrow->SetHiddenInGame(false);
	GetCapsuleComponent()->bHiddenInGame = false;
#else
	DirectionArrow->SetHiddenInGame(true);
	GetCapsuleComponent()->bHiddenInGame = true;
#endif
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	Grid = Cast<ABombermanGrid>(UGameplayStatics::GetActorOfClass(GetWorld(), ABombermanGrid::StaticClass()));

	HealthComponent->OnDeath.AddDynamic(this, &AEnemyBase::OnDeath);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnCapsuleOverlap);

	// Disable UE's built-in movement input — we drive position directly in Tick
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	SpawnDefaultController();

	// Pick initial direction and start moving
	CurrentDirection = PickRandomUnblockedDirection();
	StartMovingToNextTile();
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Grid) return;

	if (!bMovingToTile)
	{
		StartMovingToNextTile();
		return;
	}

	FVector Current = GetActorLocation();
	FVector Dir = (TargetWorldPos - Current);
	float DistRemaining = Dir.Size2D();

	// Snap to tile when close enough, then decide next move
	if (DistRemaining <= 2.f)
	{
		FVector Snapped = TargetWorldPos;
		Snapped.Z = Current.Z;
		SetActorLocation(Snapped);
		bMovingToTile = false;

		OnTileReached();
		StartMovingToNextTile();
		return;
	}

	// Move toward target
	FVector MoveDir = Dir.GetSafeNormal2D();
	AddMovementInput(MoveDir, 1.f);

	// Update arrow to face movement direction
	if (!MoveDir.IsZero())
	{
		DirectionArrow->SetWorldRotation(MoveDir.Rotation());
	}
}

void AEnemyBase::OnTileReached()
{
	// Base: pick a new random direction if current one is blocked, otherwise keep going
	if (IsDirectionBlocked(CurrentDirection))
	{
		CurrentDirection = PickRandomUnblockedDirection();
	}
}

void AEnemyBase::StartMovingToNextTile()
{
	if (!Grid || CurrentDirection.IsZero())
	{
		CurrentDirection = PickRandomUnblockedDirection();
		if (CurrentDirection.IsZero()) return;
	}

	FVector2D GridPos = Grid->GetGridPositionFromWorld(GetActorLocation());
	int32 NX = FMath::RoundToInt(GridPos.X + CurrentDirection.X);
	int32 NY = FMath::RoundToInt(GridPos.Y + CurrentDirection.Y);

	if (IsDirectionBlocked(CurrentDirection))
	{
		CurrentDirection = PickRandomUnblockedDirection();
		if (CurrentDirection.IsZero()) return;

		NX = FMath::RoundToInt(GridPos.X + CurrentDirection.X);
		NY = FMath::RoundToInt(GridPos.Y + CurrentDirection.Y);
	}

	// Release previous reservation, claim new tile
	ReleaseReservation();
	Grid->ReserveTile(NX, NY);
	CurrentReservedTile = FIntPoint(NX, NY);

	TargetWorldPos = Grid->GetTileWorldPosition(NX, NY);
	TargetWorldPos.Z = GetActorLocation().Z;
	bMovingToTile = true;
}

FVector2D AEnemyBase::PickRandomUnblockedDirection() const
{
	const TArray<FVector2D> Directions = { FVector2D(1, 0), FVector2D(-1, 0), FVector2D(0, 1), FVector2D(0, -1) };

	TArray<FVector2D> Shuffled = Directions;
	for (int32 i = Shuffled.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		Shuffled.Swap(i, j);
	}

	for (const FVector2D& Dir : Shuffled)
	{
		if (!IsDirectionBlocked(Dir)) return Dir;
	}

	return FVector2D::ZeroVector;
}

bool AEnemyBase::IsDirectionBlocked(FVector2D Dir) const
{
	if (!Grid || Dir.IsZero()) return true;

	FVector2D GridPos = Grid->GetGridPositionFromWorld(GetActorLocation());
	int32 NX = FMath::RoundToInt(GridPos.X + Dir.X);
	int32 NY = FMath::RoundToInt(GridPos.Y + Dir.Y);

	if (!Grid->IsInBounds(NX, NY)) return true;

	ETileContent Tile = Grid->GetTileContent(NX, NY);

	if (Tile == ETileContent::HardBlock) return true;
	if (Tile == ETileContent::Bomb) return true;
	if (Tile == ETileContent::SoftBlock && !bCanPassThroughSoftBlocks) return true;

	return IsNextTileOccupied(Dir);
}

bool AEnemyBase::IsNextTileOccupied(FVector2D Dir) const
{
	if (!Grid) return false;

	FVector2D GridPos = Grid->GetGridPositionFromWorld(GetActorLocation());
	int32 NX = FMath::RoundToInt(GridPos.X + Dir.X);
	int32 NY = FMath::RoundToInt(GridPos.Y + Dir.Y);

	return Grid->IsTileOccupiedByEnemy(NX, NY) || Grid->IsTileReserved(NX, NY);
}

void AEnemyBase::OnDeath()
{
	ReleaseReservation();

	if (ABombermanGameMode* GM = Cast<ABombermanGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnEnemyDied(PointValue);
	}

	Destroy();
}

void AEnemyBase::OnCapsuleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy overlap with: %s"), *OtherActor->GetName());

	if (!Cast<ABombermanCharacter>(OtherActor)) return;

	if (UBombermanHealthComponent* Health = OtherActor->FindComponentByClass<UBombermanHealthComponent>())
	{
		Health->TakeDamage(1.f);
	}
}

void AEnemyBase::ReleaseReservation()
{
	if (Grid && CurrentReservedTile != FIntPoint(-1, -1))
	{
		Grid->ReleaseTile(CurrentReservedTile.X, CurrentReservedTile.Y);
		CurrentReservedTile = FIntPoint(-1, -1);
	}
}
