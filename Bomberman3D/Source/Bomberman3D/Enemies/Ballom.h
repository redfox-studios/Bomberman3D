// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "Ballom.generated.h"

// Ballom: purely random movement, changes direction on wall hit.
// All logic lives in EnemyBase - this class just sets the speed.
UCLASS()

class BOMBERMAN3D_API ABallom : public AEnemyBase
{
	GENERATED_BODY()

  public:
	ABallom();
};
