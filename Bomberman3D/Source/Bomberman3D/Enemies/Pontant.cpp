// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Enemies/Pontant.h"
#include "Grid/BombermanGrid.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

APontant::APontant() { MoveSpeed = 200.f; }

void APontant::OnTileReached() { ChasePlayer(); }

void APontant::ChasePlayer()
{
	if (!Grid) return;

	ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!Player) return;

	FVector2D MyTile = Grid->GetGridPositionFromWorld(GetActorLocation());
	FVector2D PlayerTile = Grid->GetGridPositionFromWorld(Player->GetActorLocation());
	FVector2D Diff = PlayerTile - MyTile;

	// Try directions in order of preference toward player, including reverses as last resort
	TArray<FVector2D> Preferred;
	if (FMath::Abs(Diff.X) >= FMath::Abs(Diff.Y))
	{
		Preferred.Add(FVector2D(FMath::Sign(Diff.X), 0));
		Preferred.Add(FVector2D(0, FMath::Sign(Diff.Y)));
		Preferred.Add(FVector2D(-FMath::Sign(Diff.X), 0));
		Preferred.Add(FVector2D(0, -FMath::Sign(Diff.Y)));
	}
	else
	{
		Preferred.Add(FVector2D(0, FMath::Sign(Diff.Y)));
		Preferred.Add(FVector2D(FMath::Sign(Diff.X), 0));
		Preferred.Add(FVector2D(0, -FMath::Sign(Diff.Y)));
		Preferred.Add(FVector2D(-FMath::Sign(Diff.X), 0));
	}

	for (const FVector2D& Dir : Preferred)
	{
		if (!Dir.IsZero() && !IsDirectionBlocked(Dir))
		{
			CurrentDirection = Dir;
			return;
		}
	}

	CurrentDirection = FVector2D::ZeroVector;
}
