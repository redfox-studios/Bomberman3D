// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Player/BombermanPlayerUpgrades.h"
#include "BombermanPlayerState.generated.h"

UCLASS()

class BOMBERMAN3D_API ABombermanPlayerState : public APlayerState
{
	GENERATED_BODY()

  public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
	int32 Lives = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
	FBombermanPlayerUpgrades Upgrades;

	// UPROPERTY(BlueprintReadOnly, Category = "Player State")
	// int32 Score = 0;

	void AddScore(int32 Points) { SetScore(GetScore() + Points); }

	int32 GetCurrentScore() const { return FMath::RoundToInt(GetScore()); }

	// Derived from upgrades, don't set these directly
	int32 GetBombCount() const { return 1 + Upgrades.BombUp; }

	int32 GetBlastRadius() const { return 1 + Upgrades.FireUp; }
};
