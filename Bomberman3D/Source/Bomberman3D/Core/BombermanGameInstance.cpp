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
#if WITH_EDITOR
	LoadGame();
#endif
}
