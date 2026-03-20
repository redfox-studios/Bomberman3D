// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Door/BombermanDoor.h"
#include "Components/BoxComponent.h"
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
}

void ABombermanDoor::BeginPlay()
{
	Super::BeginPlay();
	OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ABombermanDoor::OnOverlapBegin);

	if (PortalVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PortalVFX, GetActorLocation());
	}
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
			if (EnterVFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EnterVFX, GetActorLocation());
			if (EnterSound) UGameplayStatics::PlaySoundAtLocation(this, EnterSound, GetActorLocation());
		}

		GM->OnPlayerEnteredDoor();
	}
}
