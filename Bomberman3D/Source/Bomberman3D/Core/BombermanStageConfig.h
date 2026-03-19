// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Enemies/EnemyBase.h"
#include "BombermanStageConfig.generated.h"

USTRUCT(BlueprintType)

struct FBombermanStageEnemyEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 3;
};

USTRUCT(BlueprintType)

struct FBombermanStageConfig : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBombermanStageEnemyEntry> Enemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StageTimer = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SoftBlockDensity = 0.65f;

	// calculated automatically, no need to set in DT
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UpgradeDensity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* BackgroundMusic = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBonusStage = false;
};
