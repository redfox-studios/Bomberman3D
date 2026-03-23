// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Enemies/Pass.h"
#include "Grid/BombermanGrid.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

APass::APass()
{
	MoveSpeed = 175.f;
	PointValue = 4000;
}

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

	// always try to chase, no distance limit
	FVector2D Preferred = FMath::Abs(Diff.X) >= FMath::Abs(Diff.Y)
		? FVector2D(FMath::Sign(Diff.X), 0)
		: FVector2D(0, FMath::Sign(Diff.Y));

	if (!Preferred.IsZero() && !IsDirectionBlocked(Preferred))
	{
		CurrentDirection = Preferred;
		return;
	}

	// preferred blocked, go random like Minvo instead of trying reverse like Pontant
	CurrentDirection = PickRandomUnblockedDirection();
}
