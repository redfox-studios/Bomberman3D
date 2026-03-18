// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Upgrades/BombermanUpgrade.h"
#include "Components/BoxComponent.h"
#include "Player/BombermanCharacter.h"
#include "Player/BombermanPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

ABombermanUpgrade::ABombermanUpgrade()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(RootComponent);
	OverlapBox->SetBoxExtent(FVector(40.f));
	OverlapBox->SetCollisionProfileName(TEXT("Trigger"));

#if WITH_EDITOR
	OverlapBox->bHiddenInGame = false;
#else
	OverlapBox->bHiddenInGame = true;
#endif
}

void ABombermanUpgrade::BeginPlay()
{
	Super::BeginPlay();
	OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ABombermanUpgrade::OnOverlapBegin);
}

void ABombermanUpgrade::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABombermanCharacter* Player = Cast<ABombermanCharacter>(OtherActor);
	if (!Player) return;

	ABombermanPlayerState* PS = Player->GetPlayerState<ABombermanPlayerState>();
	if (!PS) return;

	switch (UpgradeType)
	{
	case EUpgradeType::BombUp:
		PS->Upgrades.BombUp = FMath::Min(PS->Upgrades.BombUp + 1, 10);
		break;
	case EUpgradeType::FireUp:
		PS->Upgrades.FireUp = FMath::Min(PS->Upgrades.FireUp + 1, 10);
		break;
	case EUpgradeType::SpeedUp:
		PS->Upgrades.SpeedUp = FMath::Min(PS->Upgrades.SpeedUp + 1, 3);
		Player->GetCharacterMovement()->MaxWalkSpeed += Player->GetSpeedUpIncrement();
		break;
	case EUpgradeType::Invincible:
		if (UBombermanHealthComponent* Health = Player->FindComponentByClass<UBombermanHealthComponent>())
		{
			Health->bInvincible = true;
			FTimerHandle InvincibleTimer;
			Player->GetWorldTimerManager().SetTimer(
				InvincibleTimer,
				[Health]()
				{
					if (Health) Health->bInvincible = false;
				},
				30.f,
				false
			);
		}
		break;
	case EUpgradeType::WallPass:
		PS->Upgrades.bWallPass = true;
		Player->SetWallPass(true);
		break;

	case EUpgradeType::BombPass:
		PS->Upgrades.bBombPass = true;
		break;

	case EUpgradeType::FlamePass:
		PS->Upgrades.bFlamePass = true;
		break;

	case EUpgradeType::RemoteControl:
		PS->Upgrades.bRemoteControl = true;
		break;
	}

	if (PickupVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PickupVFX, GetActorLocation());
	}

	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}

	Destroy();
}
