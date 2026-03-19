// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Enemies/Pass.h"
#include "Grid/BombermanGrid.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

APass::APass() { MoveSpeed = 175.f; }

void APass::OnTileReached()
{
	if (!Grid) return;

	ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!Player)
	{
		AEnemyBase::OnTileReached();
		return;
	}

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

	// both preferred blocked, go random unlike Pontant
	CurrentDirection = PickRandomUnblockedDirection();
}
