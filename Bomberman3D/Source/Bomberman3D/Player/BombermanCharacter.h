// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/ArrowComponent.h"

#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "Bomb/BombermanBomb.h"
#include "Components/BombermanHealthComponent.h"

#include "BombermanCharacter.generated.h"

class ABombermanGrid;

UCLASS()

class BOMBERMAN3D_API ABombermanCharacter : public ACharacter
{
	GENERATED_BODY()

  public:
	ABombermanCharacter();

  protected:
	virtual void BeginPlay() override;

  public:
	// virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetWallPass(bool bEnabled);

	// --- input actions ---

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* PlaceBombAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* DetonateBombAction;

	// --- camera ---

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* Camera;

	// --- bomb ---

	UPROPERTY(EditDefaultsOnly, Category = "Bomb")
	TSubclassOf<ABombermanBomb> BombClass;

	// --- health ---

	UPROPERTY(VisibleAnywhere, Category = "Health")
	UBombermanHealthComponent* HealthComponent;

	// --- grid ---

	UPROPERTY(EditAnywhere, Category = "Grid")
	ABombermanGrid* Grid;

	UFUNCTION()
	FVector2D GetCurrentGridPosition() const;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float BaseSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SpeedUpIncrement = 50.f; // per SpeedUp level

	float GetSpeedUpIncrement() const { return SpeedUpIncrement; }

	// --- sfx ---
	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundBase* WalkSound;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundBase* PlaceBombSound;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundBase* DeathSound;

	// --- misc ---

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	UArrowComponent* DirectionArrow;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RespawnInvincibilityDuration = 2.f;

  private:
	void Move(const FInputActionValue& Value);
	void PlaceBomb(const FInputActionValue& Value);
	void DetonateBomb(const FInputActionValue& Value);

	// Tracks how many bombs are currently live in the world
	int32 ActiveBombCount = 0;

	UFUNCTION()
	void OnBombDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnDeath();

	FTimerHandle InvincibilityTimerHandle;

	TArray<ABombermanBomb*> ActiveBombs;
};
