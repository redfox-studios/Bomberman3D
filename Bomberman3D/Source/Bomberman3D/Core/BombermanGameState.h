// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BombermanGameState.generated.h"

UENUM(BlueprintType)
enum class EStageState : uint8
{
	WaitingToStart UMETA(DisplayName = "Waiting To Start"),
	InProgress UMETA(DisplayName = "In Progress"),
	StageClear UMETA(DisplayName = "Stage Clear"),
	GameOver UMETA(DisplayName = "Game Over")
};

UCLASS()

class BOMBERMAN3D_API ABombermanGameState : public AGameStateBase
{
	GENERATED_BODY()

  public:
	UPROPERTY(BlueprintReadOnly, Category = "Stage")
	EStageState StageState = EStageState::WaitingToStart;

	UPROPERTY(BlueprintReadOnly, Category = "Stage")
	float StageTimeRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stage")
	int32 CurrentStage = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Stage")
	int32 EnemiesRemaining = 0;
};
