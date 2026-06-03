// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Enemies/EnemyBase.h"
#include "Core/BombermanStageConfig.h"
#include "Core/BombermanGameState.h"

#include "BombermanGameMode.generated.h"

class ABombermanGrid;
class ABombermanGameState;
class ABombermanPlayerState;

UCLASS()

class BOMBERMAN3D_API ABombermanGameMode : public AGameModeBase
{
	GENERATED_BODY()

  public:
	ABombermanGameMode();

  protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

  public:
	// Called by BombermanCharacter when player runs out of lives
	void OnGameOver();

	// Called by enemy when it dies
	void OnEnemyDied(int32 Points);

	// Called when player enters the door
	void OnPlayerEnteredDoor();

	UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
	float StageTimerDuration = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
	int32 StartingLives = 3;

	// UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
	// TSubclassOf<AEnemyBase> DefaultEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
	int32 EnemyCount = 3;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> StageClearWidgetClass;

	UFUNCTION(BlueprintCallable)
	void AddScore(int32 Points);

	void AddStageTime(float Seconds);

	UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
	TSubclassOf<AEnemyBase> PontantClass;

	UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
	UDataTable* StageConfigTable;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
#if !UE_BUILD_SHIPPING
	bool bShowDebugInfo = true; // toggle this off before building
#endif

	UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
	int32 EnemyRushCount = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
	float StageClearDelay = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

	bool IsStageCompletable() const
	{
		return BombermanGameState && BombermanGameState->EnemiesRemaining <= 0;
	}

	bool GetIsCurrentStageBonus() const { return bCurrentStageIsBonus; }

	int32 GetLastNonBonusStage() const
	{
		if (!StageConfigTable) return TotalStages;
		TArray<FBombermanStageConfig*> AllRows;
		StageConfigTable->GetAllRows<FBombermanStageConfig>(TEXT(""), AllRows);
		int32 Last = 0;
		for (int32 i = 0; i < AllRows.Num(); i++)
		{
			if (AllRows[i] && !AllRows[i]->bBonusStage)
				Last = i + 1;
		}
		return Last;
	}

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	float DiscordUpdateDelay = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
	int32 TotalStages = 50;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameClearWidgetClass;

	USoundBase* CurrentDoorEnterSound = nullptr;

  private:
	void StartStage();
	void OnStageTimerTick();
	void OnStageTimerExpired();
	void StageClear();
	void SpawnEnemies();
	void LoadNextStage();

	bool bCurrentStageIsBonus = false;

	FTimerHandle StageTimerHandle;
	FTimerHandle StageTickHandle;
	FTimerHandle StageClearDelayHandle;

	ABombermanGrid* Grid = nullptr;
	ABombermanGameState* BombermanGameState = nullptr;

	ABombermanPlayerState* GetLocalPlayerState() const;

	TArray<FBombermanStageEnemyEntry> CurrentStageEnemies;
};
