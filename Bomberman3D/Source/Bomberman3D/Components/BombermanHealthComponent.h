// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BombermanHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))

class BOMBERMAN3D_API UBombermanHealthComponent : public UActorComponent
{
	GENERATED_BODY()

  public:
	UBombermanHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 1.f;

	// called when health changes (for hud)
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	// called when health hits 0
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Health")

	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable, Category = "Health")

	float GetCurrentHealth() const { return CurrentHealth; }

	void ResetHealth();

	UPROPERTY(BlueprintReadWrite)
	bool bInvincible = false;

  private:
	float CurrentHealth = 1.f;
	bool bIsDead = false;
};
