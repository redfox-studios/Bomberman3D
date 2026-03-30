// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BombermanPlayerUpgrades.generated.h"

USTRUCT(BlueprintType)

struct FBombermanPlayerUpgrades
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BombUp = 0; // 0-10

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FireUp = 0; // 0-10

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpeedUp = 0; // 0-3

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRemoteControl = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWallPass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBombPass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlamePass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInvincible = false;
};
