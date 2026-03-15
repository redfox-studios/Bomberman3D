// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Player/BombermanPlayerUpgrades.h"
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
	void SetStage(int32 Stage) { CurrentStage = Stage; }
	// type in console 'SetStage 5' (where 5 is the desired stage number)
	// and then type 'RestartLevel' to make changes

	virtual void Init() override;
};
