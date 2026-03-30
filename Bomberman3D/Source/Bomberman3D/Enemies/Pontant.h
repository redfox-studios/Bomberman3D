// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "Pontant.generated.h"

// Pontant: always chases the player, fastest enemy.
UCLASS()

class BOMBERMAN3D_API APontant : public AEnemyBase
{
	GENERATED_BODY()

  public:
	APontant();

  protected:
	virtual void OnTileReached() override;

  private:
	void ChasePlayer();
};
