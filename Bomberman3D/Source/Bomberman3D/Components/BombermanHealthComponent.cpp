// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Components/BombermanHealthComponent.h"

UBombermanHealthComponent::UBombermanHealthComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UBombermanHealthComponent::TakeDamage(float Amount)
{
	if (bIsDead || bInvincible || Amount <= 0.f) return;

	CurrentHealth = FMath::Max(0.f, CurrentHealth - Amount);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		bIsDead = true;
		OnDeath.Broadcast();
	}
}

void UBombermanHealthComponent::Heal(float Amount)
{
	if (bIsDead || Amount <= 0.f) return;

	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UBombermanHealthComponent::ResetHealth()
{
	bIsDead = false;
	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}
