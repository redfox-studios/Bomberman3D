// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Enemies/Onil.h"
#include "Grid/BombermanGrid.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AOnil::AOnil()
{
	MoveSpeed = 150.f;
	PointValue = 200;
}

void AOnil::OnTileReached()
{
	if (!Grid) return;

	ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (Player)
	{
		FVector2D MyTile = Grid->GetGridPositionFromWorld(GetActorLocation());
		FVector2D PlayerTile = Grid->GetGridPositionFromWorld(Player->GetActorLocation());

		if (FVector2D::Distance(MyTile, PlayerTile) <= PursuitRange)
		{
			MoveTowardPlayer();
			return;
		}
	}

	// Outside pursuit range - fall back to base random behavior
	AEnemyBase::OnTileReached();
}

void AOnil::MoveTowardPlayer()
{
	if (!Grid) return;

	ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!Player) return;

	FVector2D MyTile = Grid->GetGridPositionFromWorld(GetActorLocation());
	FVector2D PlayerTile = Grid->GetGridPositionFromWorld(Player->GetActorLocation());
	FVector2D Diff = PlayerTile - MyTile;

	TArray<FVector2D> Preferred;
	if (FMath::Abs(Diff.X) >= FMath::Abs(Diff.Y))
	{
		Preferred.Add(FVector2D(FMath::Sign(Diff.X), 0));
		Preferred.Add(FVector2D(0, FMath::Sign(Diff.Y)));
	}
	else
	{
		Preferred.Add(FVector2D(0, FMath::Sign(Diff.Y)));
		Preferred.Add(FVector2D(FMath::Sign(Diff.X), 0));
	}

	for (const FVector2D& Dir : Preferred)
	{
		if (!Dir.IsZero() && !IsDirectionBlocked(Dir))
		{
			CurrentDirection = Dir;
			return;
		}
	}

	// Both preferred blocked, go random
	CurrentDirection = PickRandomUnblockedDirection();
}
