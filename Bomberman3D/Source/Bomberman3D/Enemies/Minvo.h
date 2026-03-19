// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "Minvo.generated.h"

UCLASS()

class BOMBERMAN3D_API AMinvo : public AEnemyBase
{
	GENERATED_BODY()

  public:
	AMinvo();

  protected:
	virtual void OnTileReached() override;

	UPROPERTY(EditAnywhere, Category = "AI")
	float PursuitRange = 4.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float StuckChance = 0.25f; // 25% chance to stubbornly retry blocked direction
};
