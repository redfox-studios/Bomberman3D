// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "Pass.generated.h"

UCLASS()

class BOMBERMAN3D_API APass : public AEnemyBase
{
	GENERATED_BODY()

  public:
	APass();

  protected:
	virtual void OnTileReached() override;
};
