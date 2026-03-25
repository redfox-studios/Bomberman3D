// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Door/BombermanDoor.h"
#include "Components/BoxComponent.h"
#include "Components/MaterialBillboardComponent.h"
#include "Player/BombermanCharacter.h"
#include "Core/BombermanGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

ABombermanDoor::ABombermanDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(RootComponent);
	OverlapBox->SetBoxExtent(FVector(40.f));
	OverlapBox->SetCollisionProfileName(TEXT("Trigger"));

	MaterialBillboard = CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("MaterialBillboard"));
	MaterialBillboard->SetupAttachment(RootComponent);
}

void ABombermanDoor::BeginPlay()
{
	Super::BeginPlay();
	OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ABombermanDoor::OnOverlapBegin);

	if (PortalVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PortalVFX, GetActorLocation());
	}

	if (NearbySound)
	{
		NearbySoundComponent = UGameplayStatics::SpawnSoundAtLocation(
			this, NearbySound, GetActorLocation(), FRotator::ZeroRotator, 1.f, 1.f, 0.f, nullptr, nullptr, true
		);
	}

	if (ClosedMaterial) MaterialBillboard->SetMaterial(0, ClosedMaterial);
}

void ABombermanDoor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Door overlap: %s"), *OtherActor->GetName());
	if (bEntered) return;

	if (!Cast<ABombermanCharacter>(OtherActor)) return;

	if (ABombermanGameMode* GM = Cast<ABombermanGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GM->IsStageCompletable())
		{
			bEntered = true;

			if (ABombermanCharacter* Char = Cast<ABombermanCharacter>(OtherActor))
			{
				if (UBombermanHealthComponent* HC = Char->FindComponentByClass<UBombermanHealthComponent>())
					HC->bInvincible = true;
			}

			if (NearbySoundComponent) NearbySoundComponent->Stop();
			if (EnterVFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EnterVFX, GetActorLocation());

			// use stage-specific sound if set, otherwise fall back to default
			USoundBase* SoundToPlay = GM->CurrentDoorEnterSound ? GM->CurrentDoorEnterSound : EnterSound;
			if (SoundToPlay) UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, GetActorLocation());
		}

		GM->OnPlayerEnteredDoor();
	}
}

void ABombermanDoor::ChangeDoorColor()
{
	// https://forums.unrealengine.com/t/creating-a-dynamic-material-instance-on-c/362062
	// UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
	// DynMaterial->SetScalarParameterValue("MyParameter", myFloatValue);
	// MyComponent1->SetMaterial(0, DynMaterial);
	// MyComponent2->SetMaterial(0, DynMaterial);

	if (ABombermanGameMode* GM = Cast<ABombermanGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GM->IsStageCompletable())
		{
			if (OpenMaterial && MaterialBillboard)
			{
				MaterialBillboard->SetMaterial(0, OpenMaterial);
			}
		}
		else if (!GM->IsStageCompletable())
		{
			if (ClosedMaterial && MaterialBillboard)
			{
				MaterialBillboard->SetMaterial(0, ClosedMaterial);
			}
		}
	}
	else
	{
		if (DefaultMaterial && MaterialBillboard)
		{
			MaterialBillboard->SetMaterial(0, DefaultMaterial);
		}
	}
}
