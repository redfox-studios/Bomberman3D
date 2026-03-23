// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Enemies/Dahl.h"

ADahl::ADahl()
{
	MoveSpeed = 125.f;
	PointValue = 400;
}

void ADahl::OnTileReached()
{
	// try to keep going in current axis
	if (!IsDirectionBlocked(CurrentDirection)) return;

	// blocked - try to switch axis
	bMovingHorizontal = !bMovingHorizontal;

	TArray<FVector2D> Options;
	if (bMovingHorizontal)
		Options = { FVector2D(0, 1), FVector2D(0, -1) };
	else
		Options = { FVector2D(1, 0), FVector2D(-1, 0) };

	for (const FVector2D& Dir : Options)
	{
		if (!IsDirectionBlocked(Dir))
		{
			CurrentDirection = Dir;
			return;
		}
	}

	// both axes blocked, fall back to random
	CurrentDirection = PickRandomUnblockedDirection();
}
