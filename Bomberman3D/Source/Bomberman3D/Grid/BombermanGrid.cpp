// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Grid/BombermanGrid.h"
#include "Engine/World.h"
#include "Core/BombermanGameMode.h"
#include "Core/BombermanGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Enemies/EnemyBase.h"

ABombermanGrid::ABombermanGrid() { PrimaryActorTick.bCanEverTick = true; }

void ABombermanGrid::BeginPlay()
{
	Super::BeginPlay();

	// calculate grid size based on current stage
	// doing this because its way better than saving the grid properties
	if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		int32 Growths = (GI->CurrentStage - 1) / GridGrowthPerStages;
		BaseGridWidth = FMath::Min(BaseGridWidth + Growths * 2, MaxGridWidth);
		BaseGridHeight = FMath::Min(BaseGridHeight + Growths * 2, MaxGridHeight);
	}

	InitGrid();
	PlaceHardWalls();
	// moved: GenerateSoftBlocks, PlaceDoor, PlaceUpgrades
}

void ABombermanGrid::GenerateGrid(const FBombermanPlayerUpgrades& PlayerUpgrades, int32 CurrentStage, bool bBonusStage)
{
	GenerateSoftBlocks();
	if (!bBonusStage) PlaceDoor();
	PlaceUpgrades(PlayerUpgrades, CurrentStage);
}

void ABombermanGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bDrawDebug) return;

	for (int32 X = 0; X < BaseGridHeight; X++)
	{
		for (int32 Y = 0; Y < BaseGridWidth; Y++)
		{
			FVector Center = GetTileWorldPosition(X, Y) /* + FVector(0, 0, 50.f) */
				;
			FColor Color = Data[X][Y] == ETileContent::HardBlock ? FColor::Red
				: Data[X][Y] == ETileContent::SoftBlock			 ? FColor::Green
				: Data[X][Y] == ETileContent::Bomb				 ? FColor::Yellow
				: Data[X][Y] == ETileContent::Door				 ? FColor::Blue
																 : FColor::White;
			DrawDebugBox(GetWorld(), Center, FVector(TileSize * 0.45f), Color, false, -1.f, 0, 2.f);
		}
	}
}

void ABombermanGrid::InitGrid()
{
	Data.Empty();
	Data.Reserve(BaseGridHeight);

	ActorMap.Empty();
	ActorMap.Reserve(BaseGridHeight);

	for (int32 i = 0; i < BaseGridHeight; i++)
	{
		TArray<ETileContent> Row;
		Row.Init(ETileContent::Empty, BaseGridWidth);
		Data.Add(Row);

		TArray<AActor*> ActorRow;
		ActorRow.Init(nullptr, BaseGridWidth);
		ActorMap.Add(ActorRow);
	}

	UpgradeMap.Empty();
	UpgradeMap.Reserve(BaseGridHeight);

	for (int32 i = 0; i < BaseGridHeight; i++)
	{
		TArray<TSubclassOf<AActor>> Row;
		Row.Init(nullptr, BaseGridWidth);
		UpgradeMap.Add(Row);
	}
}

void ABombermanGrid::PlaceHardWalls()
{
	for (int32 X = 0; X < BaseGridHeight; X++)
	{
		for (int32 Y = 0; Y < BaseGridWidth; Y++)
		{
			bool bIsBorder = (X == 0 || X == BaseGridHeight - 1 || Y == 0 || Y == BaseGridWidth - 1);
			bool bIsPillar = (X % 2 == 0 && Y % 2 == 0);

			if (bIsBorder || bIsPillar)
			{
				Data[X][Y] = ETileContent::HardBlock;

				if (HardBlockClass)
				{
					SpawnActorOnTile(X, Y, HardBlockClass);
				}
			}
		}
	}
}

void ABombermanGrid::GenerateSoftBlocks()
{
	if (!SoftBlockClass) return;

	// Player spawns at top-left interior tile (1,1)
	// Safe zone: no soft blocks within PlayerSafeZone tiles of spawn
	const FVector2D SpawnTile = GetPlayerSpawnTile();
	const int32 SpawnX = FMath::RoundToInt(SpawnTile.X);
	const int32 SpawnY = FMath::RoundToInt(SpawnTile.Y);

	// old
	//const int32 SpawnX = 1;
	//const int32 SpawnY = 1;

	for (int32 X = 1; X < BaseGridHeight - 1; X++)
	{
		for (int32 Y = 1; Y < BaseGridWidth - 1; Y++)
		{
			// Skip hard blocks
			if (Data[X][Y] == ETileContent::HardBlock) continue;

			// Skip player safe zone
			if (FMath::Abs(X - SpawnX) <= PlayerSafeZone && FMath::Abs(Y - SpawnY) <= PlayerSafeZone) continue;

			// Random density check
			if (FMath::FRand() <= SoftBlockDensity)
			{
				Data[X][Y] = ETileContent::SoftBlock;
				UE_LOG(LogTemp, Log, TEXT("SoftBlock at [%d,%d]"), X, Y);
				SpawnActorOnTile(X, Y, SoftBlockClass);
			}
		}
	}
}

void ABombermanGrid::PlaceDoor()
{
	// Collect all soft block tiles
	TArray<FVector2D> SoftTiles;
	for (int32 X = 1; X < BaseGridHeight - 1; X++)
	{
		for (int32 Y = 1; Y < BaseGridWidth - 1; Y++)
		{
			if (Data[X][Y] == ETileContent::SoftBlock)
			{
				SoftTiles.Add(FVector2D(X, Y));
			}
		}
	}

	if (SoftTiles.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlaceDoor: no soft blocks to hide door under!"));
		return;
	}

	// Shuffle and try candidates until we find one where door is reachable
	// Fisher-Yates shuffle
	for (int32 i = SoftTiles.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		SoftTiles.Swap(i, j);
	}

	for (const FVector2D& Candidate : SoftTiles)
	{
		int32 CX = FMath::RoundToInt(Candidate.X);
		int32 CY = FMath::RoundToInt(Candidate.Y);

		if (IsDoorReachable(CX, CY))
		{
			DoorTileX = CX;
			DoorTileY = CY;
			// Door is hidden - tile stays SoftBlock visually until destroyed
			UE_LOG(LogTemp, Log, TEXT("Door hidden at [%d, %d]"), DoorTileX, DoorTileY);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("PlaceDoor: could not find a reachable door position!"));
}

bool ABombermanGrid::IsDoorReachable(int32 DoorX, int32 DoorY) const
{
	// Temporarily treat the door tile as empty for the flood fill check
	// We do this by passing the door coords to FloodFill and checking if it reaches them

	// Player spawn is always (1, 1)
	TArray<FVector2D> Reachable = FloodFill(1, 1);

	// Check if door tile is in reachable set
	// (treat it as passable since destroying the soft block reveals it)
	for (const FVector2D& Tile : Reachable)
	{
		// Adjacent to door tile counts as reachable
		int32 TX = FMath::RoundToInt(Tile.X);
		int32 TY = FMath::RoundToInt(Tile.Y);

		if ((FMath::Abs(TX - DoorX) == 1 && TY == DoorY) || (FMath::Abs(TY - DoorY) == 1 && TX == DoorX))
		{
			return true;
		}
	}

	return false;
}

TArray<FVector2D> ABombermanGrid::FloodFill(int32 StartX, int32 StartY) const
{
	TArray<FVector2D> Visited;
	TSet<FVector2D> VisitedSet;
	TQueue<FVector2D> Queue;

	FVector2D Start(StartX, StartY);
	Queue.Enqueue(Start);
	VisitedSet.Add(Start);

	const FVector2D Dirs[] = { FVector2D(1, 0), FVector2D(-1, 0), FVector2D(0, 1), FVector2D(0, -1) };

	while (!Queue.IsEmpty())
	{
		FVector2D Current;
		Queue.Dequeue(Current);
		Visited.Add(Current);

		for (const FVector2D& Dir : Dirs)
		{
			int32 NX = FMath::RoundToInt(Current.X + Dir.X);
			int32 NY = FMath::RoundToInt(Current.Y + Dir.Y);

			if (!IsInBounds(NX, NY)) continue;
			if (Data[NX][NY] == ETileContent::HardBlock) continue;
			if (Data[NX][NY] == ETileContent::SoftBlock) continue;

			FVector2D Next(NX, NY);
			if (VisitedSet.Contains(Next)) continue;

			VisitedSet.Add(Next);
			Queue.Enqueue(Next);
		}
	}

	return Visited;
}

// isinbounds credits to: https://github.com/kubgus
bool ABombermanGrid::IsInBounds(int32 X, int32 Y) const { return X >= 0 && X < BaseGridHeight && Y >= 0 && Y < BaseGridWidth; }

bool ABombermanGrid::IsTileWalkable(int32 X, int32 Y) const
{
	if (!IsInBounds(X, Y)) return false;

	ETileContent Tile = Data[X][Y];
	return Tile != ETileContent::SoftBlock && Tile != ETileContent::HardBlock && Tile != ETileContent::Bomb;
}

FVector ABombermanGrid::GetTileWorldPosition(int32 X, int32 Y) const { return GetActorLocation() + FVector(X * TileSize, Y * TileSize, TileSize * 0.5f); }

ETileContent ABombermanGrid::GetTileContent(int32 X, int32 Y) const
{
	if (!IsInBounds(X, Y)) return ETileContent::HardBlock;

	return Data[X][Y];
}

void ABombermanGrid::SetTileContent(int32 X, int32 Y, ETileContent NewContent)
{
	if (!IsInBounds(X, Y)) return;
	Data[X][Y] = NewContent;
}

bool ABombermanGrid::IsTileSoft(int32 X, int32 Y) const
{
	if (!IsInBounds(X, Y)) return false;
	return Data[X][Y] == ETileContent::SoftBlock;
}

FVector2D ABombermanGrid::GetGridPositionFromWorld(FVector WorldLocation) const
{
	FVector LocalPos = WorldLocation - GetActorLocation();
	return FVector2D(FMath::RoundToInt(LocalPos.X / TileSize), FMath::RoundToInt(LocalPos.Y / TileSize));
}

FVector ABombermanGrid::GetPlayerSpawnPosition() const
{
	// Player always spawns at tile (1, 1) - top-left interior
	return GetTileWorldPosition(1, 1);
}

AActor* ABombermanGrid::SpawnActorOnTile(int32 X, int32 Y, TSubclassOf<AActor> ActorClass)
{
	if (!IsInBounds(X, Y) || !ActorClass) return nullptr;

	if (ActorMap[X][Y] != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnActorOnTile: tile [%d, %d] already has an actor"), X, Y);
		return nullptr;
	}

	FVector WorldPos = GetTileWorldPosition(X, Y);
	AActor* Spawned = GetWorld()->SpawnActor<AActor>(ActorClass, WorldPos, FRotator::ZeroRotator);

	if (Spawned)
	{
		ActorMap[X][Y] = Spawned;
	}

	return Spawned;
}

void ABombermanGrid::DestroyActorOnTile(int32 X, int32 Y)
{
	if (!IsInBounds(X, Y)) return;

	AActor* Actor = ActorMap[X][Y];
	if (!Actor) return;

	// If this was the door tile, spawn the door actor now
	if (X == DoorTileX && Y == DoorTileY && DoorClass)
	{
		Data[X][Y] = ETileContent::Door;
		ActorMap[X][Y] = nullptr;
		Actor->Destroy();
		SpawnActorOnTile(X, Y, DoorClass);
		return;
	}

	Actor->Destroy();
	ActorMap[X][Y] = nullptr;
	Data[X][Y] = ETileContent::Empty;

	if (ABombermanGameMode* GM = Cast<ABombermanGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->AddScore(10);
	}

	// spawn upgrade if hidden here
	if (UpgradeMap[X][Y])
	{
		SpawnActorOnTile(X, Y, UpgradeMap[X][Y]);
		UpgradeMap[X][Y] = nullptr;
	}
}

AActor* ABombermanGrid::GetActorOnTile(int32 X, int32 Y) const
{
	if (!IsInBounds(X, Y)) return nullptr;
	return ActorMap[X][Y];
}

void ABombermanGrid::PlaceUpgrades(const FBombermanPlayerUpgrades& PlayerUpgrades, int32 CurrentStage)
{
	// calculated automatically, no need to set in DT
	UpgradeDensity = FMath::Clamp(CurrentStage * 0.01f, 0.01f, 0.5f); // '0.5 divided by 50 = 0.01' where 0.5 is the max upgrade density, 50 is the number of stages and 0.01 is upgrade density increasing by each stage

	for (int32 X = 1; X < BaseGridHeight - 1; X++)
	{
		for (int32 Y = 1; Y < BaseGridWidth - 1; Y++)
		{
			if (Data[X][Y] != ETileContent::SoftBlock) continue;
			if (X == DoorTileX && Y == DoorTileY) continue;
			if (FMath::FRand() > UpgradeDensity) continue;

			TArray<TSubclassOf<AActor>> Pool;

			// always available
			if (PlayerUpgrades.BombUp < 10 && BombUpClass) Pool.Add(BombUpClass);
			if (PlayerUpgrades.FireUp < 10 && FireUpClass) Pool.Add(FireUpClass);
			if (PlayerUpgrades.SpeedUp < 3 && SpeedUpClass && CurrentStage >= 4) Pool.Add(SpeedUpClass);

			// mid game
			if (CurrentStage >= 6)
			{
				if (!PlayerUpgrades.bRemoteControl && RemoteControlClass) Pool.Add(RemoteControlClass);
				if (!PlayerUpgrades.bBombPass && BombPassClass) Pool.Add(BombPassClass);
			}

			// late game
			if (CurrentStage >= 16)
			{
				if (!PlayerUpgrades.bWallPass && WallPassClass) Pool.Add(WallPassClass);
				if (!PlayerUpgrades.bFlamePass && FlamePassClass) Pool.Add(FlamePassClass);
				if (!PlayerUpgrades.bInvincible && InvincibleClass) Pool.Add(InvincibleClass);
			}

			if (Pool.IsEmpty()) continue;
			UpgradeMap[X][Y] = Pool[FMath::RandRange(0, Pool.Num() - 1)];
		}
	}
}

bool ABombermanGrid::IsTileOccupiedByEnemy(int32 X, int32 Y) const
{
	if (!IsInBounds(X, Y)) return false;

	FVector TileWorld = GetTileWorldPosition(X, Y);
	float HalfTile = TileSize * 0.5f;

	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };

	UKismetSystemLibrary::BoxOverlapActors(GetWorld(), TileWorld, FVector(HalfTile * 0.6f), ObjectTypes, AEnemyBase::StaticClass(), TArray<AActor*>{}, OverlappingActors);

	return OverlappingActors.Num() > 0;
}

void ABombermanGrid::ReserveTile(int32 X, int32 Y) { ReservedTiles.Add(FIntPoint(X, Y)); }

void ABombermanGrid::ReleaseTile(int32 X, int32 Y) { ReservedTiles.Remove(FIntPoint(X, Y)); }

bool ABombermanGrid::IsTileReserved(int32 X, int32 Y) const { return ReservedTiles.Contains(FIntPoint(X, Y)); }
