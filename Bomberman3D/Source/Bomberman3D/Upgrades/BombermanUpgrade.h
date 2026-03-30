// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "Components/BoxComponent.h" // because i set box collision as root
#include "BombermanUpgrade.generated.h"

// clang-format off
UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	BombUp			UMETA(DisplayName = "Bomb Up"),
	FireUp			UMETA(DisplayName = "Fire Up"),
	SpeedUp			UMETA(DisplayName = "Speed Up"),
	Invincible		UMETA(DisplayName = "Invincible"),
	WallPass		UMETA(DisplayName = "Wall Pass"),
	BombPass		UMETA(DisplayName = "Bomb Pass"),
	FlamePass		UMETA(DisplayName = "Flame Pass"),
	RemoteControl	UMETA(DisplayName = "Remote Control")
};
// clang-format on

UCLASS()

class BOMBERMAN3D_API ABombermanUpgrade : public AActor
{
	GENERATED_BODY()

  public:
	ABombermanUpgrade();

  protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

  public:
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	EUpgradeType UpgradeType = EUpgradeType::BombUp;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* PickupVFX;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundBase* PickupSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float FloatAmplitude = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float FloatSpeed = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float RotateSpeed = 90.f; // degrees per second

	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	float InvincibleDuration = 30.f;

  private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* OverlapBox;

	float FloatTime = 0.f;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
