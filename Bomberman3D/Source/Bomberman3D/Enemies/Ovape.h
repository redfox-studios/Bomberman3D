// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "Ovape.generated.h"

UCLASS()

class BOMBERMAN3D_API AOvape : public AEnemyBase
{
	GENERATED_BODY()

  public:
	AOvape();

  protected:
	virtual void OnTileReached() override;
	virtual bool IsDirectionBlocked(FVector2D Dir) const override;

	UPROPERTY(EditAnywhere, Category = "AI")
	float ChaseChance = 0.1f; // 10% chance to chase player each tile
};
