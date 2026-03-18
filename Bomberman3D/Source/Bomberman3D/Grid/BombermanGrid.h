// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/BombermanPlayerUpgrades.h"
#include "BombermanGrid.generated.h"

UENUM(BlueprintType)
enum class ETileContent : uint8
{
	Empty UMETA(DisplayName = "Empty"),
	SoftBlock UMETA(DisplayName = "Soft Block"),
	HardBlock UMETA(DisplayName = "Hard Block"),
	Bomb UMETA(DisplayName = "Bomb"),
	Upgrade UMETA(DisplayName = "Upgrade"),
	Door UMETA(DisplayName = "Door")
};

UCLASS()

class BOMBERMAN3D_API ABombermanGrid : public AActor
{
	GENERATED_BODY()

  public:
	ABombermanGrid();

  protected:
	virtual void BeginPlay() override;

	// --- config ---

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	int32 BaseGridWidth = 13;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	int32 BaseGridHeight = 11;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	int32 GridGrowthPerStages = 10;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	int32 MaxGridWidth = 21;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	int32 MaxGridHeight = 17;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	float TileSize = 100.f;

	// How many tiles around player spawn are guaranteed empty (from top-left corner)
	UPROPERTY(EditAnywhere, Category = "Grid Config")
	int32 PlayerSafeZone = 2;

	// Actor classes - assign in editor
	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> HardBlockClass;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> SoftBlockClass;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> DoorClass;

  public:
	virtual void Tick(float DeltaTime) override;

	// --- tile queries ---

	UFUNCTION(BlueprintCallable)
	bool IsInBounds(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable)
	bool IsTileWalkable(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable)
	bool IsTileSoft(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable)
	ETileContent GetTileContent(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable)
	void SetTileContent(int32 X, int32 Y, ETileContent NewContent);

	UFUNCTION(BlueprintCallable)
	FVector GetTileWorldPosition(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable)
	FVector2D GetGridPositionFromWorld(FVector WorldLocation) const;

	int32 GetGridWidth() const { return BaseGridWidth; }

	int32 GetGridHeight() const { return BaseGridHeight; }

	int32 GetPlayerSafeZone() const { return PlayerSafeZone; }

	bool IsTileOccupiedByEnemy(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable)
	void GenerateGrid(const FBombermanPlayerUpgrades& PlayerUpgrades, int32 CurrentStage);

	// --- actor spawning ---

	UFUNCTION(BlueprintCallable)
	AActor* SpawnActorOnTile(int32 X, int32 Y, TSubclassOf<AActor> ActorClass);

	UFUNCTION(BlueprintCallable)
	void DestroyActorOnTile(int32 X, int32 Y);

	UFUNCTION(BlueprintCallable)
	AActor* GetActorOnTile(int32 X, int32 Y) const;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> BombUpClass;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> FireUpClass;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> SpeedUpClass;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> InvincibleClass;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> WallPassClass;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> BombPassClass;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> FlamePassClass;

	UPROPERTY(EditAnywhere, Category = "Grid Config")
	TSubclassOf<AActor> RemoteControlClass;

	// 0.0 - 1.0, how many soft blocks hide an upgrade
	UPROPERTY(EditAnywhere, Category = "Grid Config")
	float UpgradeDensity = 0.2f;

	// 0.0 - 1.0, how dense soft blocks are
	UPROPERTY(EditAnywhere, Category = "Grid Config")
	float SoftBlockDensity = 0.65f;

	TArray<TArray<TSubclassOf<AActor>>> UpgradeMap;

	// --- misc ---

	float GetTileSize() const { return TileSize; }

	// Returns player spawn position in world space (top-left safe zone)
	FVector GetPlayerSpawnPosition() const;

	FVector2D GetPlayerSpawnTile() const { return FVector2D(1, 1); }

	// Debug
	UPROPERTY(EditAnywhere, Category = "Grid Debug")
	bool bDrawDebug = false;

	void ReserveTile(int32 X, int32 Y);
	void ReleaseTile(int32 X, int32 Y);
	bool IsTileReserved(int32 X, int32 Y) const;

  private:
	TSet<FIntPoint> ReservedTiles;

  private:
	void InitGrid();
	void PlaceHardWalls();
	void GenerateSoftBlocks();
	void PlaceDoor();
	void PlaceUpgrades(const FBombermanPlayerUpgrades& PlayerUpgrades, int32 CurrentStage);

	// Flood-fill from player spawn, returns all reachable empty tiles
	TArray<FVector2D> FloodFill(int32 StartX, int32 StartY) const;

	// Check if door tile is reachable from player spawn
	// (temporarily treats the soft block hiding the door as empty)
	bool IsDoorReachable(int32 DoorX, int32 DoorY) const;

	TArray<TArray<ETileContent>> Data;
	TArray<TArray<AActor*>> ActorMap;

	// Where the door is hidden (under a soft block)
	int32 DoorTileX = -1;
	int32 DoorTileY = -1;
};
