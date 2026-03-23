// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Enemies/Ovape.h"
#include "Grid/BombermanGrid.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AOvape::AOvape()
{
	MoveSpeed = 125.f;
	PointValue = 2000;
	bCanPassThroughSoftBlocks = true;
}

void AOvape::OnTileReached()
{
	if (FMath::FRand() <= ChaseChance)
	{
		ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (Player && Grid)
		{
			FVector2D MyTile = Grid->GetGridPositionFromWorld(GetActorLocation());
			FVector2D PlayerTile = Grid->GetGridPositionFromWorld(Player->GetActorLocation());
			FVector2D Diff = PlayerTile - MyTile;

			FVector2D Preferred = FMath::Abs(Diff.X) >= FMath::Abs(Diff.Y)
				? FVector2D(FMath::Sign(Diff.X), 0)
				: FVector2D(0, FMath::Sign(Diff.Y));

			if (!Preferred.IsZero() && !IsDirectionBlocked(Preferred))
			{
				CurrentDirection = Preferred;
				return;
			}
		}
	}

	// default random behavior
	AEnemyBase::OnTileReached();
}
