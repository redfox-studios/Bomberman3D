// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "Dahl.generated.h"

UCLASS()

class BOMBERMAN3D_API ADahl : public AEnemyBase
{
	GENERATED_BODY()

  public:
	ADahl();

  protected:
	virtual void OnTileReached() override;

  private:
	bool bMovingHorizontal = true;
};
