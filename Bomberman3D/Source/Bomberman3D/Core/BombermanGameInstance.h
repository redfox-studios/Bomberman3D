// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Player/BombermanPlayerUpgrades.h"
#include "Core/BombermanDiscordManager.h"
#include "Components/AudioComponent.h"

#include "BombermanGameInstance.generated.h"

UCLASS()

class BOMBERMAN3D_API UBombermanGameInstance : public UGameInstance
{
	GENERATED_BODY()

  public:
	// static constexpr int32 DefaultStage = 1;
	// static constexpr int32 DefaultLives = 3;

	UPROPERTY(BlueprintReadWrite, Category = "Game Instance")
	int32 CurrentStage = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Game Instance")
	int32 Lives = 3;

	UPROPERTY(BlueprintReadWrite, Category = "Game Instance")
	FBombermanPlayerUpgrades Upgrades;

	UFUNCTION(BlueprintCallable, Category = "Game Instance")
	void ResetToDefaults();

	UPROPERTY(BlueprintReadWrite, Category = "Game Instance")
	int32 Score = 0;

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame();

	UFUNCTION(BlueprintCallable, Exec)
	void SetStage(int32 Stage);
	// type in console 'SetStage 5' (where 5 is the desired stage number)

	FBombermanDiscordManager DiscordManager;

	virtual void Init() override;
	virtual void OnStart() override;
	virtual void Shutdown() override;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	UAudioComponent* MusicComponent = nullptr;

	void PlayMusic(USoundBase* Music, float Volume = 0.5f);
	void StopMusic();

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	float MusicFadeDuration = 1.f;

	void FadeToMusic(USoundBase* Music, float Volume = 0.5f);

	UFUNCTION(BlueprintCallable)
	void StopMusicImmediate();

	// --- settings ---

	UFUNCTION(BlueprintCallable)
	void SaveSettings();

	UFUNCTION(BlueprintCallable)
	void LoadSettings();

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	USoundClass* MusicSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	USoundClass* SFXSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	USoundClass* AmbienceSoundClass;

	UFUNCTION(BlueprintCallable)
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable)
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintCallable)
	void SetAmbienceVolume(float Volume);

	UFUNCTION(BlueprintCallable)
	void ApplySoundSettings();

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float MusicVolume = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float SFXVolume = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float AmbienceVolume = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	int32 ResolutionWidth = 1920;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	int32 ResolutionHeight = 1080;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	int32 WindowMode = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	int32 QualityPreset = 3;

	UFUNCTION(BlueprintCallable)
	void SetResolution(int32 Width, int32 Height);

	UFUNCTION(BlueprintCallable)
	void SetWindowMode(int32 Mode);

	UFUNCTION(BlueprintCallable)
	void SetQualityPreset(int32 Preset);

	UFUNCTION(BlueprintCallable)
	void ApplyVideoSettings();

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	bool bMuteOnFocusLost = false;

	UFUNCTION(BlueprintCallable)
	void SetMuteOnFocusLost(bool bEnabled);
};
