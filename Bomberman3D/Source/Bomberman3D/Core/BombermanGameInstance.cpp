// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Core/BombermanGameInstance.h"
#include "Core/BombermanSaveGame.h"
#include "Kismet/GameplayStatics.h"

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

	UE_LOG(LogTemp, Warning, TEXT("Loaded: Stage=%d Lives=%d Score=%d SpeedUp=%d BombUp=%d FireUp=%d"), CurrentStage, Lives, Score, Upgrades.SpeedUp, Upgrades.BombUp, Upgrades.FireUp);
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
	if (!Music) return;

	if (MusicComponent && MusicComponent->Sound == Music && MusicComponent->IsPlaying())
		return;

	if (MusicComponent)
		MusicComponent->Stop();

	MusicComponent = UGameplayStatics::SpawnSound2D(
		GetWorld(), Music, Volume, 1.f, 0.f, nullptr, true, true // bPersistAcrossLevelTransition = true
	);
}

void UBombermanGameInstance::StopMusic()
{
	if (MusicComponent) MusicComponent->Stop();
}
