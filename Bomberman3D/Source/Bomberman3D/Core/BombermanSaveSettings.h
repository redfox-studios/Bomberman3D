// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BombermanSaveSettings.generated.h"

UCLASS()

class BOMBERMAN3D_API UBombermanSaveSettings : public USaveGame
{
	GENERATED_BODY()

  public:
	UPROPERTY()
	float MusicVolume = 1.f;

	UPROPERTY()
	float SFXVolume = 1.f;

	UPROPERTY()
	float UIVolume = 1.f;

	UPROPERTY()
	float AmbienceVolume = 1.f;

	UPROPERTY()
	bool bMuteOnFocusLost = false;

	UPROPERTY()
	int32 ResolutionWidth = 1920;

	UPROPERTY()
	int32 ResolutionHeight = 1080;

	UPROPERTY()
	int32 WindowMode = 0; // EWindowMode - Fullscreen, Borderless, Windowed - https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/ApplicationCore/EWindowMode__Type

	UPROPERTY()
	int32 QualityPreset = 3; // 0-3 (Low, Medium, High, Epic)
};
