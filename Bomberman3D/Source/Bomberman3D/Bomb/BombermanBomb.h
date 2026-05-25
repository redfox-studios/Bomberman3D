// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "NiagaraSystem.h"
#include "BombermanBomb.generated.h"

// Placed -> Armed -> Detonating -> Explosion -> Cleanup
//
// Placed:     bomb is placed after triggering the IA
// Armed:      player steps off the tile
// Detonating: fuse timer expires OR chain reaction hit
// Explosion:  actual explosion logic runs
// Cleanup:    free timer, destroy actor

// clang-format off
UENUM(BlueprintType)
enum class EBombState : uint8
{
	Placed		UMETA(DisplayName = "Placed"),
	Armed		UMETA(DisplayName = "Armed"),
	Detonating	UMETA(DisplayName = "Detonating"),
	Explosion	UMETA(DisplayName = "Explosion"),
	Cleanup		UMETA(DisplayName = "Cleanup")
};
// clang-format on

class ABombermanGrid;

UCLASS()

class BOMBERMAN3D_API ABombermanBomb : public AActor
{
	GENERATED_BODY()

  public:
	ABombermanBomb();

  protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

  public:
	UPROPERTY(EditDefaultsOnly, Category = "Bomb")
	float FuseTimer = 2.5f;

	// Tile count. 1 = default bomberman blast range.
	UPROPERTY(EditDefaultsOnly, Category = "Bomb")
	int32 BlastRadius = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Bomb")
	EBombState CurrentState = EBombState::Placed;

	UPROPERTY(VisibleAnywhere, Category = "Bomb")
	UStaticMeshComponent* BombMesh;

	UPROPERTY(VisibleAnywhere, Category = "Fire Point (leave empty)")
	USceneComponent* FirePoint;

	UPROPERTY(EditAnywhere, Category = "Grid")
	ABombermanGrid* Grid;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* ExplosionVFX;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* IgnitionVFX;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundBase* ExplosionSound;

	ACharacter* OwnerCharacter = nullptr;

	FTimerHandle FuseTimerHandle;

	UFUNCTION()
	void Detonate();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb")
	void OnChainDetonated();

	static void ResetExplosionSoundTimer() { LastExplosionSoundTime = -999.f; }

	UPROPERTY(EditDefaultsOnly, Category = "Bomb")
	float ExplosionSoundCooldown = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Bomb")
	float CollisionEnableDistance = 0.65f;

  private:
	void Explode();
	void TriggerChainReaction(int32 X, int32 Y);
	void DamageActorsOnTile(int32 X, int32 Y);
	bool bCollisionEnabled = false;
	static float LastExplosionSoundTime;
	UNiagaraComponent* IgnitionNiagara = nullptr;
};
