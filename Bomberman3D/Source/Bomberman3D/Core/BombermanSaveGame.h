// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Player/BombermanPlayerUpgrades.h"
#include "BombermanSaveGame.generated.h"

UCLASS()

class BOMBERMAN3D_API UBombermanSaveGame : public USaveGame
{
	GENERATED_BODY()

  public:
	UPROPERTY()
	int32 CurrentStage = 1;

	UPROPERTY()
	int32 Lives = 3;

	UPROPERTY()
	int32 Score = 0;

	UPROPERTY()
	FBombermanPlayerUpgrades Upgrades;

	UPROPERTY()
	float MusicVolume = 1.f;

	UPROPERTY()
	float SFXVolume = 1.f;

	UPROPERTY()
	float AmbienceVolume = 1.f;
};
