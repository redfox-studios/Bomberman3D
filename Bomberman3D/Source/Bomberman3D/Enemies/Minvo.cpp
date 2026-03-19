// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Enemies/Minvo.h"
#include "Grid/BombermanGrid.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AMinvo::AMinvo() { MoveSpeed = 115.f; }

void AMinvo::OnTileReached()
{
	if (!Grid) return;

	// stubbornness check - retry current direction even if blocked
	if (IsDirectionBlocked(CurrentDirection) && FMath::FRand() <= StuckChance)
		return; // keep current direction, will fail to move and retry next tile

	ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (Player)
	{
		FVector2D MyTile = Grid->GetGridPositionFromWorld(GetActorLocation());
		FVector2D PlayerTile = Grid->GetGridPositionFromWorld(Player->GetActorLocation());

		if (FVector2D::Distance(MyTile, PlayerTile) <= PursuitRange)
		{
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
