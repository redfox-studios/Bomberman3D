// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Bomb/BombermanBomb.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Grid/BombermanGrid.h"
#include "Components/BombermanHealthComponent.h"
#include "EngineUtils.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Player/BombermanPlayerState.h"

float ABombermanBomb::LastExplosionSoundTime = -999.f;

ABombermanBomb::ABombermanBomb()
{
	PrimaryActorTick.bCanEverTick = true;

	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bomb Mesh"));
	RootComponent = BombMesh;

	BombMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("Fire Point")); //where the ignition niagara spawns
	FirePoint->SetupAttachment(RootComponent);
}

void ABombermanBomb::BeginPlay()
{
	Super::BeginPlay();

	Grid = Cast<ABombermanGrid>(UGameplayStatics::GetActorOfClass(GetWorld(), ABombermanGrid::StaticClass()));

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), IgnitionVFX, FirePoint->GetComponentLocation());

	GetWorld()->GetTimerManager().SetTimer(FuseTimerHandle, this, &ABombermanBomb::Detonate, FuseTimer, false);
}

void ABombermanBomb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCollisionEnabled || !Grid) return;

	if (OwnerCharacter)
	{
		if (ABombermanPlayerState* PS = OwnerCharacter->GetPlayerState<ABombermanPlayerState>())
		{
			if (PS->Upgrades.bBombPass) return;
		}
	}

	if (!OwnerCharacter) return;

	FVector PlayerPos = OwnerCharacter->GetActorLocation();
	FVector BombPos = GetActorLocation();

	// Use actual distance instead of tile rounding - player must be
	// at least 60% of a tile away before we enable collision
	float Dist2D = FVector2D::Distance(
		FVector2D(PlayerPos.X, PlayerPos.Y),
		FVector2D(BombPos.X, BombPos.Y)
	);

	if (Dist2D > Grid->GetTileSize() * CollisionEnableDistance)
	{
		bCollisionEnabled = true;
		BombMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}

void ABombermanBomb::Detonate()
{
	if (CurrentState == EBombState::Detonating || CurrentState == EBombState::Explosion || CurrentState == EBombState::Cleanup) return;

	if (!Grid) return;

	CurrentState = EBombState::Detonating;

	// Clear timer - handles the case where we got chain-triggered before fuse expired
	GetWorld()->GetTimerManager().ClearTimer(FuseTimerHandle);
	Explode();
}

void ABombermanBomb::Explode()
{
	CurrentState = EBombState::Explosion;

	FVector2D BombGridPos = Grid->GetGridPositionFromWorld(GetActorLocation());
	int32 BX = FMath::RoundToInt(BombGridPos.X);
	int32 BY = FMath::RoundToInt(BombGridPos.Y);

	// Clear our tile first - prevents other explosion rays from trying to chain back into us
	Grid->SetTileContent(BX, BY, ETileContent::Empty);

	// Damage anything on the bomb's own tile
	DamageActorsOnTile(BX, BY);

	if (ExplosionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVFX, Grid->GetTileWorldPosition(BX, BY));
	}

	const TArray<FVector2D> Directions = { FVector2D(1, 0), FVector2D(-1, 0), FVector2D(0, 1), FVector2D(0, -1) };

	// only play sound if no other explosion played in the last 0.1s
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (ExplosionSound && CurrentTime - LastExplosionSoundTime > ExplosionSoundCooldown)
	{
		LastExplosionSoundTime = CurrentTime;
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	}
	
	for (const FVector2D& Dir : Directions)
	{
		for (int32 i = 1; i <= BlastRadius; i++)
		{
			int32 X = BX + FMath::RoundToInt(Dir.X * i);
			int32 Y = BY + FMath::RoundToInt(Dir.Y * i);

			ETileContent Tile = Grid->GetTileContent(X, Y);
			UE_LOG(LogTemp, Warning, TEXT("Checking tile [%d, %d] = %d"), X, Y, (int32)Tile);

			// FVector Center = FVector(Dir.X, Dir.Y, 50.f);
			DrawDebugBox(GetWorld(), Grid->GetTileWorldPosition(X, Y) + FVector(0.f, 0.f, Grid->GetTileSize() * 0.5f), FVector(Grid->GetTileSize() * 0.25f), FColor::Purple, false, 2.f, 0, 2.f);

			if (Tile == ETileContent::HardBlock)
			{
				break;
			}
			else if (Tile == ETileContent::SoftBlock)
			{
				Grid->DestroyActorOnTile(X, Y);
				DamageActorsOnTile(X, Y);

				if (ExplosionVFX)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVFX, Grid->GetTileWorldPosition(X, Y) + FVector(0.f, 0.f, Grid->GetTileSize() * 0.5f));
				}

				break;
			}
			else if (Tile == ETileContent::Bomb)
			{
				UE_LOG(LogTemp, Warning, TEXT("Chain reaction triggered at [%d, %d]"), X, Y);
				Grid->SetTileContent(X, Y, ETileContent::Empty);
				TriggerChainReaction(X, Y);
				break;
			}
			else
			{
				// Empty tile - damage anything standing here (player, enemy)
				DamageActorsOnTile(X, Y);

				if (ExplosionVFX)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVFX, Grid->GetTileWorldPosition(X, Y) + FVector(0.f, 0.f, Grid->GetTileSize() * 0.5f));
				}
			}
		}
	}

	CurrentState = EBombState::Cleanup;
	Destroy();
}

void ABombermanBomb::DamageActorsOnTile(int32 X, int32 Y)
{
	UE_LOG(LogTemp, Warning, TEXT("Looking for bomb actor at [%d, %d]"), X, Y);
	if (!Grid) return;

	FVector TileWorld = Grid->GetTileWorldPosition(X, Y);
	float HalfTile = Grid->GetTileSize() * 0.5f;

	// Overlap check - find all actors within this tile's bounds
	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };

	UKismetSystemLibrary::BoxOverlapActors(
		GetWorld(),
		TileWorld,
		FVector(HalfTile * 0.9f), // slightly smaller than tile to avoid edge bleed
		ObjectTypes,
		nullptr,
		TArray<AActor*>{ this },
		OverlappingActors
	);

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor) continue;

		if (Actor == OwnerCharacter)
		{
			ABombermanPlayerState* PS = OwnerCharacter->GetPlayerState<ABombermanPlayerState>();
			if (PS && PS->Upgrades.bFlamePass) continue;
		}

		UBombermanHealthComponent* Health = Actor->FindComponentByClass<UBombermanHealthComponent>();
		if (Health) Health->TakeDamage(1.f);
	}
}

void ABombermanBomb::TriggerChainReaction(int32 X, int32 Y)
{
	for (TActorIterator<ABombermanBomb> It(GetWorld()); It; ++It)
	{
		ABombermanBomb* OtherBomb = *It;
		if (OtherBomb == this) continue;

		FVector2D OtherGridPos = Grid->GetGridPositionFromWorld(OtherBomb->GetActorLocation());
		if (FMath::RoundToInt(OtherGridPos.X) == X && FMath::RoundToInt(OtherGridPos.Y) == Y)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found bomb actor, detonating"));
			OtherBomb->OnChainDetonated();
			OtherBomb->Detonate();
			return;
		}
	}
}
