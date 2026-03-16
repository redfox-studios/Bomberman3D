// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Player/BombermanCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"

#include "Grid/BombermanGrid.h"
#include "Bomb/BombermanBomb.h"
#include "Player/BombermanPlayerState.h"
#include "Core/BombermanGameMode.h"
#include "Core/BombermanGameInstance.h"
#include "Core/BombermanGameState.h"

#include "Bomberman3D.h"

#include "GameFramework/CharacterMovementComponent.h"

ABombermanCharacter::ABombermanCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->SetCapsuleSize(30.f,
										  60.f); // UE defaults -> 36 , 80

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bInheritYaw = false;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetRelativeRotation(FRotator(-65.f, 0.f, 0.f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	HealthComponent = CreateDefaultSubobject<UBombermanHealthComponent>(TEXT("HealthComponent"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

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

void ABombermanCharacter::BeginPlay()
{
	Super::BeginPlay();

	Grid = Cast<ABombermanGrid>(UGameplayStatics::GetActorOfClass(GetWorld(), ABombermanGrid::StaticClass()));
	HealthComponent->OnDeath.AddDynamic(this, &ABombermanCharacter::OnDeath);

	GetWorld()->GetTimerManager().SetTimerForNextTick(
		[this]()
		{
			if (ABombermanPlayerState* PS = GetPlayerState<ABombermanPlayerState>())
			{
				if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(GetGameInstance()))
				{
					UE_LOG(LogTemp, Warning, TEXT("GI state: Lives=%d BombUp=%d FireUp=%d SpeedUp=%d"), GI->Lives, GI->Upgrades.BombUp, GI->Upgrades.FireUp, GI->Upgrades.SpeedUp);

					PS->Lives = GI->Lives;
					PS->Upgrades = GI->Upgrades;
					PS->SetScore(GI->Score);
				}
				GetCharacterMovement()->MaxWalkSpeed = BaseSpeed + (PS->Upgrades.SpeedUp * SpeedUpIncrement);
				SetWallPass(PS->Upgrades.bWallPass);
			}
			else
			{
				GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
			}
		}
	);
}

void ABombermanCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABombermanCharacter::Move);
		EnhancedInput->BindAction(PlaceBombAction, ETriggerEvent::Started, this, &ABombermanCharacter::PlaceBomb);
		EnhancedInput->BindAction(DetonateBombAction, ETriggerEvent::Started, this, &ABombermanCharacter::DetonateBomb);
	}
}

void ABombermanCharacter::Move(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	AddMovementInput(FVector::ForwardVector, Input.Y);
	AddMovementInput(FVector::RightVector, Input.X);

	if (WalkSound && !GetCharacterMovement()->Velocity.IsZero())
	{
		UGameplayStatics::PlaySoundAtLocation(this, WalkSound, GetActorLocation());
	}
}

void ABombermanCharacter::PlaceBomb(const FInputActionValue& Value)
{
	if (!Grid || !BombClass) return;

	ABombermanPlayerState* PS = GetPlayerState<ABombermanPlayerState>();

	UE_LOG(LogTemp, Warning, TEXT("PlaceBomb called. ActiveBombCount: %d, MaxBombs: %d"), ActiveBombCount, PS ? PS->GetBombCount() : -1);

	FVector2D GridPos = GetCurrentGridPosition();
	int32 GX = FMath::RoundToInt(GridPos.X);
	int32 GY = FMath::RoundToInt(GridPos.Y);

	ETileContent CurrentTile = Grid->GetTileContent(GX, GY);
	// if (CurrentTile == ETileContent::Bomb) return;
	// if (CurrentTile != ETileContent::Empty && (!PS || !PS->Upgrades.bBombPass)) return;
	if (CurrentTile == ETileContent::Bomb && (!PS || !PS->Upgrades.bBombPass)) return;
	if (CurrentTile != ETileContent::Empty && CurrentTile != ETileContent::Bomb && CurrentTile != ETileContent::SoftBlock) return;
	if (CurrentTile == ETileContent::SoftBlock && (!PS || !PS->Upgrades.bWallPass)) return;

	if (PS && ActiveBombCount >= PS->GetBombCount()) return;

	if (CurrentTile == ETileContent::SoftBlock)
	{
		Grid->DestroyActorOnTile(GX, GY);
	}

	FVector WorldPos = Grid->GetTileWorldPosition(GX, GY);
	ABombermanBomb* Bomb = GetWorld()->SpawnActor<ABombermanBomb>(BombClass, WorldPos, FRotator::ZeroRotator);
	if (!Bomb) return;

	if (PlaceBombSound) UGameplayStatics::PlaySoundAtLocation(this, PlaceBombSound, GetActorLocation());

	Bomb->OwnerCharacter = this;
	if (PS) Bomb->BlastRadius = PS->GetBlastRadius();

	Grid->SetTileContent(GX, GY, ETileContent::Bomb);
	ActiveBombCount++;

	ActiveBombs.Add(Bomb);
	Bomb->OnDestroyed.AddDynamic(this, &ABombermanCharacter::OnBombDestroyed);

	UE_LOG(LogTemp, Warning, TEXT("Bomb placed at [%d, %d]"), GX, GY);
}

void ABombermanCharacter::OnBombDestroyed(AActor* DestroyedActor)
{
	ActiveBombCount = FMath::Max(0, ActiveBombCount - 1);
	ActiveBombs.Remove(Cast<ABombermanBomb>(DestroyedActor));
}

void ABombermanCharacter::OnDeath()
{
	ABombermanPlayerState* PS = GetPlayerState<ABombermanPlayerState>();
	if (!PS) return;

	// decrement lives
	PS->Lives--;

	if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(GetGameInstance()))
	{
		if (ABombermanGameState* GS = GetWorld()->GetGameState<ABombermanGameState>())
		{
			GI->DiscordManager.UpdatePresence(GS->CurrentStage, PS->Lives, false);
		}
	}

	// reset upgrades except bombup & fireup
	// PS->Upgrades.BombUp = 0;
	// PS->Upgrades.FireUp = 0;
	PS->Upgrades.SpeedUp = 0;
	PS->Upgrades.bRemoteControl = false;
	PS->Upgrades.bWallPass = false;
	PS->Upgrades.bBombPass = false;
	PS->Upgrades.bFlamePass = false;
	PS->Upgrades.bInvincible = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	SetWallPass(false);

	UE_LOG(LogTemp, Warning, TEXT("Player died. Lives remaining: %d"), PS->Lives);

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	if (PS->Lives <= 0)
	{
		if (ABombermanGameMode* GM = Cast<ABombermanGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->OnGameOver();
		}
		return;
	}

	// Still has lives - reset and respawn
	HealthComponent->ResetHealth();
	SetActorLocation(Grid ? Grid->GetPlayerSpawnPosition() : FVector::ZeroVector);

	HealthComponent->bInvincible = true;

	GetWorld()->GetTimerManager().SetTimer(InvincibilityTimerHandle, [this]()
										   { HealthComponent->bInvincible = false; },
										   2.f,
										   false);
}

FVector2D ABombermanCharacter::GetCurrentGridPosition() const
{
	if (Grid) return Grid->GetGridPositionFromWorld(GetActorLocation());

	return FVector2D::ZeroVector;
}

void ABombermanCharacter::DetonateBomb(const FInputActionValue& Value)
{
	ABombermanPlayerState* PS = GetPlayerState<ABombermanPlayerState>();
	if (!PS || !PS->Upgrades.bRemoteControl) return;

	if (ActiveBombs.Num() == 0) return;

	ABombermanBomb* Oldest = ActiveBombs[0];
	if (Oldest) Oldest->Detonate();
}

void ABombermanCharacter::SetWallPass(bool bEnabled)
{
	ECollisionResponse Response = bEnabled ? ECR_Ignore : ECR_Block;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_SoftBlock, Response);
}
