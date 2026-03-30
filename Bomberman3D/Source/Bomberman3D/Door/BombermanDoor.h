// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "BombermanDoor.generated.h"

UCLASS()

class BOMBERMAN3D_API ABombermanDoor : public AActor
{
	GENERATED_BODY()

  public:
	ABombermanDoor();

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* PortalVFX;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* EnterVFX;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundBase* EnterSound;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundBase* NearbySound;

	UPROPERTY()
	UAudioComponent* NearbySoundComponent;

	UFUNCTION(BlueprintCallable)
	void ChangeDoorColor();

	// materials
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UMaterialInterface* ClosedMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UMaterialInterface* OpenMaterial;

  protected:
	virtual void BeginPlay() override;

  private:
	bool bEntered = false;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* OverlapBox;

	UPROPERTY(VisibleAnywhere)
	class UMaterialBillboardComponent* MaterialBillboard;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
