// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "Onil.generated.h"

// Onil: random movement, switches to pursuing player when within PursuitRange tiles.
UCLASS()

class BOMBERMAN3D_API AOnil : public AEnemyBase
{
	GENERATED_BODY()

  public:
	AOnil();

  protected:
	virtual void OnTileReached() override;

	UPROPERTY(EditAnywhere, Category = "AI")
	float PursuitRange = 5.f; // in tiles

  private:
	void MoveTowardPlayer();
};
