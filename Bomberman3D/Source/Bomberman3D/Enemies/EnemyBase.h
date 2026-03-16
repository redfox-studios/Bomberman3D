// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BombermanHealthComponent.h"
#include "Components/ArrowComponent.h"
#include "EnemyBase.generated.h"

class ABombermanGrid;

UCLASS(Abstract)

class BOMBERMAN3D_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

  public:
	AEnemyBase();

  protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Called when the enemy reaches its target tile - override in subclasses
	// Base implementation: pick a random unblocked direction
	virtual void OnTileReached();

	// Returns true if moving to the adjacent tile in Dir is not allowed
	virtual bool IsDirectionBlocked(FVector2D Dir) const;

	// Picks a random unblocked direction, returns ZeroVector if all blocked
	FVector2D PickRandomUnblockedDirection() const;

	bool IsNextTileOccupied(FVector2D Dir) const;

	// Current movement direction (in grid units)
	FVector2D CurrentDirection = FVector2D::ZeroVector;

	FIntPoint CurrentReservedTile = FIntPoint(-1, -1);
	void ReleaseReservation();

  public:
	UPROPERTY(VisibleAnywhere, Category = "Health")
	UBombermanHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeed = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	UArrowComponent* DirectionArrow;

	ABombermanGrid* Grid = nullptr;

  private:
	// Tile-to-tile movement state
	FVector TargetWorldPos = FVector::ZeroVector;
	bool bMovingToTile = false;

	void StartMovingToNextTile();

	UFUNCTION()
	void OnDeath();

	UFUNCTION()
	void OnCapsuleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
