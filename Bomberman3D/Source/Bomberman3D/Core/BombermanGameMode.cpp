// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Core/BombermanGameMode.h"
#include "Core/BombermanGameState.h"
#include "Core/BombermanGameInstance.h"
#include "Core/BombermanStageConfig.h"

#include "Player/BombermanPlayerController.h"
#include "Player/BombermanCharacter.h"
#include "Player/BombermanPlayerState.h"

#include "Enemies/EnemyBase.h"

#include "Grid/BombermanGrid.h"

#include "Components/CapsuleComponent.h"

#include "Bomb/BombermanBomb.h"
#include "Door/BombermanDoor.h"

// --- ac ---
#include "AntiCheat/BombermanAntiCheat.h"

// --- engine ---
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ABombermanGameMode::ABombermanGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultPawnClass = ABombermanCharacter::StaticClass();
	PlayerControllerClass = ABombermanPlayerController::StaticClass();
	PlayerStateClass = ABombermanPlayerState::StaticClass();
	GameStateClass = ABombermanGameState::StaticClass();
}

void ABombermanGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ac checks
	FBombermanAntiCheat::RunChecks();

	if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(GetGameInstance()))
	{
		GI->DiscordManager.RunCallbacks();
	}

	// --- DEBUG ---

	if (!bShowDebugInfo) return;

	// ABombermanPlayerState* PS = nullptr;
	ABombermanPlayerState* PS = GetLocalPlayerState();

	if (BombermanGameState)
	{
		GEngine->AddOnScreenDebugMessage(
			0,
			0.f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Stage: %d | State: %d | Timer: %.0f | Enemies: %d"),
				BombermanGameState->CurrentStage,
				(int32)BombermanGameState->StageState,
				BombermanGameState->StageTimeRemaining,
				BombermanGameState->EnemiesRemaining
			)
		);
	}

	if (PS)
	{
		GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Cyan, FString::Printf(TEXT("Lives: %d | Score: %d"), PS->Lives, (int32)PS->GetScore()));

		GEngine->AddOnScreenDebugMessage(
			2,
			0.f,
			FColor::Green,
			FString::Printf(
				TEXT("BombUp: %d | FireUp: %d | SpeedUp: %d | Invincible: %d | WallPass: %d | BombPass: %d | FlamePass: %d | RemoteControl: %d"),
				PS->Upgrades.BombUp,
				PS->Upgrades.FireUp,
				PS->Upgrades.SpeedUp,
				PS->Upgrades.bInvincible,
				PS->Upgrades.bWallPass,
				PS->Upgrades.bBombPass,
				PS->Upgrades.bFlamePass,
				PS->Upgrades.bRemoteControl
			)
		);
	}
}

void ABombermanGameMode::BeginPlay()
{
	Super::BeginPlay();

	Grid = Cast<ABombermanGrid>(UGameplayStatics::GetActorOfClass(GetWorld(), ABombermanGrid::StaticClass()));
	BombermanGameState = GetGameState<ABombermanGameState>();

	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT("BombermanGameMode: no grid found in level!"));
		return;
	}

	StartStage();
}

void ABombermanGameMode::StartStage()
{
	if (!BombermanGameState) return;

	ABombermanBomb::ResetExplosionSoundTimer();

	if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(GetGameInstance()))
	{
		BombermanGameState->CurrentStage = GI->CurrentStage;
		// GI->CurrentStage++;

		// ABombermanPlayerState* PS = GetLocalPlayerState();

		FTimerHandle DiscordUpdateHandle;
		GetWorld()->GetTimerManager().SetTimer(DiscordUpdateHandle, [this]()
		{
			if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(GetGameInstance()))
			{
				ABombermanPlayerState* PS = GetLocalPlayerState();
				GI->DiscordManager.UpdatePresence(BombermanGameState->CurrentStage, PS ? PS->Lives : 3, PS ? PS->GetCurrentScore() : 0, false);
			}
		}, DiscordUpdateDelay, false);

		/*	
		// now managed in characters beginplay
		for (TActorIterator<ABombermanCharacter> It(GetWorld()); It; ++It)
		{
			if (ABombermanPlayerState* PS = It->GetPlayerState<ABombermanPlayerState>())
			{
				PS->Lives = GI->Lives;
				PS->Upgrades = GI->Upgrades;
				PS->SetScore(GI->Score);

				float Speed = It->BaseSpeed + (PS->Upgrades.SpeedUp * It->GetSpeedUpIncrement());
				It->GetCharacterMovement()->MaxWalkSpeed = Speed;
			}
			break;
		}
		*/
	}

	if (StageConfigTable)
	{
		TArray<FBombermanStageConfig*> AllRows;
		StageConfigTable->GetAllRows<FBombermanStageConfig>(TEXT(""), AllRows);

		int32 StageIndex = FMath::Clamp(BombermanGameState->CurrentStage - 1, 0, AllRows.Num() - 1);
		FBombermanStageConfig* Config = AllRows[StageIndex];

		if (Config)
		{
			CurrentStageEnemies = Config->Enemies;
			StageTimerDuration = Config->StageTimer;
			Grid->SoftBlockDensity = Config->SoftBlockDensity;
			Grid->UpgradeDensity = Config->UpgradeDensity;
			bCurrentStageIsBonus = Config->bBonusStage;
			CurrentDoorEnterSound = Config->DoorEnterSound;

			if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(GetGameInstance()))
			{
				if (Config->BackgroundMusic)
				{
					UE_LOG(LogTemp, Warning, TEXT("Playing music: %s"), *Config->BackgroundMusic->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("No music set for this stage"));
				}

				GI->FadeToMusic(Config->BackgroundMusic);
			}
		}
	}

	FBombermanPlayerUpgrades CurrentUpgrades;
	if (ABombermanPlayerState* PS = GetLocalPlayerState())
	{
		CurrentUpgrades = PS->Upgrades;
	}

	Grid->GenerateGrid(CurrentUpgrades, BombermanGameState->CurrentStage, bCurrentStageIsBonus);

	BombermanGameState->StageState = EStageState::InProgress;
	BombermanGameState->StageTimeRemaining = StageTimerDuration;
	BombermanGameState->EnemiesRemaining = 0;

	// Move player to spawn
	for (TActorIterator<ABombermanCharacter> It(GetWorld()); It; ++It)
	{
		It->SetActorLocation(Grid->GetPlayerSpawnPosition());
		break;
	}

	// Instead of calling SpawnEnemies directly
	FTimerHandle SpawnDelay;
	GetWorld()->GetTimerManager().SetTimer(SpawnDelay, this, &ABombermanGameMode::SpawnEnemies, 0.1f, false);

	// change door color
	FTimerHandle DoorColorHandle;
	GetWorld()->GetTimerManager().SetTimer(DoorColorHandle, [this]()
	{ 
		if (!BombermanGameState) return;
		if (IsStageCompletable())
		{
			AActor* DoorActor = UGameplayStatics::GetActorOfClass(GetWorld(), ABombermanDoor::StaticClass());
			if (ABombermanDoor* BD = Cast<ABombermanDoor>(DoorActor))
				BD->ChangeDoorColor();
		}
	}, 0.2f, false);

	// Tick timer every second
	GetWorld()->GetTimerManager().SetTimer(StageTickHandle, this, &ABombermanGameMode::OnStageTimerTick, 1.f, true);

	// Full stage timer
	GetWorld()->GetTimerManager().SetTimer(StageTimerHandle, this, &ABombermanGameMode::OnStageTimerExpired, StageTimerDuration, false);

	UE_LOG(LogTemp, Log, TEXT("Stage %d started. Enemies: %d"), BombermanGameState->CurrentStage, BombermanGameState->EnemiesRemaining);
}

void ABombermanGameMode::SpawnEnemies()
{
	if (!Grid || !BombermanGameState) return;

	TArray<FVector2D> ValidTiles;
	int32 SafeZone = Grid->GetPlayerSafeZone();
	FVector2D SpawnTile = Grid->GetPlayerSpawnTile();

	for (int32 X = 1; X < Grid->GetGridHeight() - 1; X++)
	{
		for (int32 Y = 1; Y < Grid->GetGridWidth() - 1; Y++)
		{
			if (Grid->GetTileContent(X, Y) != ETileContent::Empty) continue;
			if (FMath::Abs(X - SpawnTile.X) <= SafeZone && FMath::Abs(Y - SpawnTile.Y) <= SafeZone) continue;
			ValidTiles.Add(FVector2D(X, Y));
		}
	}

	// Shuffle once
	for (int32 i = ValidTiles.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		ValidTiles.Swap(i, j);
	}

	int32 TileIndex = 0;
	for (const FBombermanStageEnemyEntry& Entry : CurrentStageEnemies)
	{
		if (!Entry.EnemyClass) continue;

		AEnemyBase* CDO = Entry.EnemyClass->GetDefaultObject<AEnemyBase>();
		float HalfHeight = CDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		for (int32 i = 0; i < Entry.Count; i++)
		{
			if (TileIndex >= ValidTiles.Num()) break;

			FVector WorldPos = Grid->GetTileWorldPosition(FMath::RoundToInt(ValidTiles[TileIndex].X), FMath::RoundToInt(ValidTiles[TileIndex].Y));
			WorldPos.Z = HalfHeight;
			TileIndex++;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AEnemyBase* Enemy = GetWorld()->SpawnActor<AEnemyBase>(Entry.EnemyClass, WorldPos, FRotator::ZeroRotator, SpawnParams);
			if (Enemy)
			{
				BombermanGameState->EnemiesRemaining++;
			}
		}
	}
}

void ABombermanGameMode::OnStageTimerTick()
{
	if (!BombermanGameState) return;
	BombermanGameState->StageTimeRemaining = FMath::Max(0.f, BombermanGameState->StageTimeRemaining - 1.f);
}

void ABombermanGameMode::OnStageTimerExpired()
{
	if (bCurrentStageIsBonus)
	{
		if (CurrentDoorEnterSound) UGameplayStatics::PlaySound2D(this, CurrentDoorEnterSound);
		StageClear();
		return;
	}

	if (!Grid || !PontantClass || !BombermanGameState) return;

	for (int32 i = 0; i < EnemyRushCount; i++)
	{
		// pick a random empty tile
		TArray<FVector2D> ValidTiles;
		for (int32 X = 1; X < Grid->GetGridHeight() - 1; X++)
		{
			for (int32 Y = 1; Y < Grid->GetGridWidth() - 1; Y++)
			{
				if (Grid->GetTileContent(X, Y) == ETileContent::Empty) ValidTiles.Add(FVector2D(X, Y));
			}
		}

		if (ValidTiles.IsEmpty()) break;

		FVector2D Tile = ValidTiles[FMath::RandRange(0, ValidTiles.Num() - 1)];
		FVector WorldPos = Grid->GetTileWorldPosition(FMath::RoundToInt(Tile.X), FMath::RoundToInt(Tile.Y));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AEnemyBase* Pontant = GetWorld()->SpawnActor<AEnemyBase>(PontantClass, WorldPos, FRotator::ZeroRotator, SpawnParams);
		if (Pontant)
		{
			BombermanGameState->EnemiesRemaining++;
		}
	}
}

void ABombermanGameMode::OnEnemyDied(int32 Points)
{
	if (!BombermanGameState) return;

	AddScore(Points);
	BombermanGameState->EnemiesRemaining = FMath::Max(0, BombermanGameState->EnemiesRemaining - 1);

	UE_LOG(LogTemp, Log, TEXT("Enemy died. Remaining: %d"), BombermanGameState->EnemiesRemaining);

	if (BombermanGameState->EnemiesRemaining <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("All enemies dead - find the door!"));

		// info for door
		AActor* BombermanDoor = UGameplayStatics::GetActorOfClass(GetWorld(), ABombermanDoor::StaticClass());
		ABombermanDoor* BD = Cast<ABombermanDoor>(BombermanDoor);

		// ABombermanDoor* BD = Cast<ABombermanDoor>(UGameplayStatics::GetActorOfClass(GetWorld(), ABombermanDoor::StaticClass())

		if(BD)
		{
			BD->ChangeDoorColor();
		}
	}
}

void ABombermanGameMode::OnPlayerEnteredDoor()
{
	if (!BombermanGameState) return;
	if (BombermanGameState->EnemiesRemaining > 0) return;

	StageClear();
}

void ABombermanGameMode::StageClear()
{
	if (!BombermanGameState) return;

	UE_LOG(LogTemp, Warning, TEXT("StageClear called"));

	BombermanGameState->StageState = EStageState::StageClear;

	GetWorld()->GetTimerManager().ClearTimer(StageTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(StageTickHandle);

	for (TActorIterator<ABombermanCharacter> It(GetWorld()); It; ++It)
	{
		It->DisableInput(nullptr);

		if (ABombermanPlayerState* PS = It->GetPlayerState<ABombermanPlayerState>())
		{
			PS->AddScore(FMath::RoundToInt(BombermanGameState->StageTimeRemaining) * 10);
			PS->Lives++;

			if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(GetGameInstance()))
			{
				GI->CurrentStage = BombermanGameState->CurrentStage + 1;
				GI->Lives = PS->Lives;
				GI->Upgrades = PS->Upgrades; // save BEFORE reset
				GI->Score = PS->GetScore();
				GI->SaveGame();
			}

			// PS->Upgrades.SpeedUp = 0;
			// PS->Upgrades.bRemoteControl = false;
			// PS->Upgrades.bWallPass = false;
			// PS->Upgrades.bBombPass = false;
			// PS->Upgrades.bFlamePass = false;
			// PS->Upgrades.bInvincible = false;
			// It->SetWallPass(false);
			// It->GetCharacterMovement()->MaxWalkSpeed = It->BaseSpeed;
		}
		break;
	}

	if (BombermanGameState->CurrentStage >= TotalStages)
	{
		if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(GetGameInstance()))
		{
			GI->ResetToDefaults();
			GI->SaveGame();
		}

		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && GameClearWidgetClass)
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(PC, GameClearWidgetClass);
			if (Widget)
			{
				Widget->AddToViewport();
				PC->bShowMouseCursor = true;
			}
		}

		return;
	}

	if (StageClearWidgetClass)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(PC, StageClearWidgetClass);
			if (Widget) Widget->AddToViewport();
		}
	}

	// FTimerHandle StageClearDelay;
	// GetWorld()->GetTimerManager().SetTimer(StageClearDelay, this, &ABombermanGameMode::LoadNextStage, 3.f, false);

	// FTimerHandle StageClearDelayHandle; - moved to .h
	GetWorld()->GetTimerManager().SetTimer(StageClearDelayHandle, this, &ABombermanGameMode::LoadNextStage, StageClearDelay, false);
}

void ABombermanGameMode::AddScore(int32 Points)
{
	if (ABombermanPlayerState* PS = GetLocalPlayerState())
	{
		PS->AddScore(Points);

		if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(GetGameInstance()))
		{
			GI->DiscordManager.UpdatePresence(BombermanGameState->CurrentStage, PS->Lives, PS->GetCurrentScore(), false);
		}
	}
}

void ABombermanGameMode::LoadNextStage()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && LoadingScreenWidgetClass)
	{
		UUserWidget* Widget = CreateWidget<UUserWidget>(PC, LoadingScreenWidgetClass);
		if (Widget) Widget->AddToViewport(999); // on top of everything
	}

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.UUID = 1;
	LatentInfo.Linkage = 0;
	LatentInfo.ExecutionFunction = NAME_None;

	UGameplayStatics::LoadStreamLevel(this, FName(*GetWorld()->GetName()), true, true, LatentInfo);
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void ABombermanGameMode::OnGameOver()
{
	if (!BombermanGameState) return;

	BombermanGameState->StageState = EStageState::GameOver;

	GetWorld()->GetTimerManager().ClearTimer(StageTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(StageTickHandle);

	// Reset GameInstance so next run starts fresh
	if (UBombermanGameInstance* GI = Cast<UBombermanGameInstance>(GetGameInstance()))
	{
		GI->ResetToDefaults();
		GI->SaveGame();
	}

	for (TActorIterator<ABombermanCharacter> It(GetWorld()); It; ++It)
	{
		It->DisableInput(nullptr);
		break;
	}

	// Show game over widget
	if (GameOverWidgetClass)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
			if (Widget)
			{
				Widget->AddToViewport();
				PC->bShowMouseCursor = true;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Game over!"));
}

ABombermanPlayerState* ABombermanGameMode::GetLocalPlayerState() const
{
	for (TActorIterator<ABombermanCharacter> It(GetWorld()); It; ++It)
	{
		return It->GetPlayerState<ABombermanPlayerState>();
	}
	return nullptr;
}
