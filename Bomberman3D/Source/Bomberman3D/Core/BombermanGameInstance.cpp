// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Core/BombermanGameInstance.h"
#include "Core/BombermanSaveGame.h"
#include "Core/BombermanSaveSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"

static const FString SaveSlot = TEXT("BombermanSave");

void UBombermanGameInstance::SaveGame()
{
	UBombermanSaveGame* Save = Cast<UBombermanSaveGame>(UGameplayStatics::CreateSaveGameObject(UBombermanSaveGame::StaticClass()));

	Save->CurrentStage = CurrentStage;
	Save->Lives = Lives;
	Save->Score = Score;
	Save->Upgrades = Upgrades;

	UGameplayStatics::SaveGameToSlot(Save, SaveSlot, 0);
}

void UBombermanGameInstance::LoadGame()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlot, 0)) return;

	UBombermanSaveGame* Save = Cast<UBombermanSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));

	if (!Save) return;

	CurrentStage = Save->CurrentStage;
	Lives = Save->Lives;
	Score = Save->Score;
	Upgrades = Save->Upgrades;

	// UE_LOG(LogTemp, Warning, TEXT("Loaded: Stage=%d Lives=%d Score=%d SpeedUp=%d BombUp=%d FireUp=%d"), CurrentStage, Lives, Score, Upgrades.SpeedUp, Upgrades.BombUp, Upgrades.FireUp);
}

void UBombermanGameInstance::ResetToDefaults()
{
	CurrentStage = 1;
	Lives = 3;
	Score = 0;
	Upgrades = FBombermanPlayerUpgrades();
}

void UBombermanGameInstance::Init()
{
	Super::Init();
	LoadGame();
	LoadSettings();
	ApplySoundSettings();
}

void UBombermanGameInstance::OnStart()
{
	Super::OnStart();
	DiscordManager.Init(1482825420733808791LL); // appID. IMPORTANT - KEEP THE 'LL' AT THE END (LONG LONG)
	DiscordManager.UpdatePresence(1, 3, 0, true);
	UE_LOG(LogTemp, Warning, TEXT("[Discord] OnStart called"));
}

void UBombermanGameInstance::Shutdown()
{
	DiscordManager.Shutdown();
	Super::Shutdown();
}

void UBombermanGameInstance::SetStage(int32 Stage)
{
	CurrentStage = Stage;
	// GEngine->Exec(GetWorld(), TEXT("RestartLevel"));
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void UBombermanGameInstance::PlayMusic(USoundBase* Music, float Volume)
{
	if (!Music)
	{
		StopMusic();
		return;
	}

	if (MusicComponent && MusicComponent->Sound == Music && MusicComponent->IsPlaying())
		return;

	if (MusicComponent)
		MusicComponent->Stop();

	MusicComponent = UGameplayStatics::SpawnSound2D(
		GetWorld(), Music, Volume, 1.f, 0.f, nullptr, true, true
	);
}

void UBombermanGameInstance::StopMusic()
{
	if (MusicComponent) MusicComponent->Stop();
}

void UBombermanGameInstance::FadeToMusic(USoundBase* Music, float Volume)
{
	if (!Music)
	{
		if (MusicComponent)
			MusicComponent->FadeOut(MusicFadeDuration, 0.f);
		return;
	}

	if (MusicComponent && MusicComponent->Sound == Music && MusicComponent->IsPlaying())
		return;

	if (MusicComponent)
		MusicComponent->FadeOut(MusicFadeDuration, 0.f);

	MusicComponent = UGameplayStatics::SpawnSound2D(
		GetWorld(), Music, Volume, 1.f, 0.f, nullptr, true, true
	);
}

void UBombermanGameInstance::StopMusicImmediate()
{
	if (MusicComponent)
	{
		MusicComponent->Stop();
		MusicComponent = nullptr;
	}
}

void UBombermanGameInstance::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.f, 1.f);
	if (MusicSoundClass)
		MusicSoundClass->Properties.Volume = MusicVolume;
}

void UBombermanGameInstance::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.f, 1.f);
	if (SFXSoundClass)
		SFXSoundClass->Properties.Volume = SFXVolume;
}

void UBombermanGameInstance::SetAmbienceVolume(float Volume)
{
	AmbienceVolume = FMath::Clamp(Volume, 0.f, 1.f);
	if (AmbienceSoundClass)
		AmbienceSoundClass->Properties.Volume = AmbienceVolume;
}

void UBombermanGameInstance::ApplySoundSettings()
{
	SetMusicVolume(MusicVolume);
	SetSFXVolume(SFXVolume);
	SetAmbienceVolume(AmbienceVolume);
}

static const FString SettingsSlot = TEXT("BombermanSettings");

void UBombermanGameInstance::SaveSettings()
{
	UBombermanSaveSettings* Save = Cast<UBombermanSaveSettings>(
		UGameplayStatics::CreateSaveGameObject(UBombermanSaveSettings::StaticClass())
	);
	Save->MusicVolume = MusicVolume;
	Save->SFXVolume = SFXVolume;
	Save->AmbienceVolume = AmbienceVolume;
	UGameplayStatics::SaveGameToSlot(Save, SettingsSlot, 0);
}

void UBombermanGameInstance::LoadSettings()
{
	if (!UGameplayStatics::DoesSaveGameExist(SettingsSlot, 0)) return;

	UBombermanSaveSettings* Save = Cast<UBombermanSaveSettings>(
		UGameplayStatics::LoadGameFromSlot(SettingsSlot, 0)
	);
	if (!Save) return;

	MusicVolume = Save->MusicVolume;
	SFXVolume = Save->SFXVolume;
	AmbienceVolume = Save->AmbienceVolume;
}
