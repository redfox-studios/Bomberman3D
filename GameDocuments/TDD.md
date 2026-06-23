# Technical Design Document - Bomberman 3D
Project: Bomberman3D<br />
Engine: Unreal Engine 5 (Hybrid C++ / Blueprints)<br />
Team: Maximilián Repa (Game Design, 3D Art, Sound), Michal Flaška (Programming), Eduard Fabo (3D Art, 2D Art)<br />
Target Platform: PC (Windows)<br />
Last Updated: `2026-06-23`

Made fully by me, grammar fixed with AI.

<br><br>

## 1. Tech Stack

| Thing            | What we're using                                    |
|------------------|-----------------------------------------------------|
| Engine           | UE5                                                 |
| Code             | C++ + Blueprints (hybrid)                           |
| Version Control  | GitHub (no LFS because we are poor)                 |
| IDE              | Visual Studio                                       |
| 3D               | Blender                                             |
| Texture/Material | Imphenzia Pixpal (one texture, one material policy) |

<br><br>

## 2. Architecture Overview

The codebase follows a hybrid approach - core systems (grid, bomb logic, AI, game state) are in C++, while designers can tweak everything through exposed `UPROPERTY` variables in the editor. Blueprints are used for things that don't need raw performance - UI, VFX triggers, simple animations.

Rule of thumb: if it has logic, it's C++. If it's just connecting things visually or needs to be tweaked by a non-coder, it can be BP.

**Custom trace channel:** `ECC_SoftBlock` is mapped to `ECC_GameTraceChannel1` (defined in `Bomberman3D.h`). This allows soft blocks to have their own collision channel, enabling WallPass to selectively ignore them without affecting other collision logic.

<br><br>

## 3. Grid System

The map is tile-based but player and enemy movement is smooth (free movement, no snapping). The grid exists purely as a data structure - it tracks what's on each tile.

### Tile Content Enum

```cpp
UENUM(BlueprintType)
enum class ETileContent : uint8
{
    Empty, SoftBlock, HardBlock, Bomb, Upgrade, Door, TopBlock
};
```

`TopBlock` is a decorative block variant placed on outer map for visual polish. It does not affect gameplay logic.

### Bomb Snap

When the player presses place bomb, we take their world position and round it to the nearest tile center:

```cpp
FVector2D ABombermanCharacter::GetCurrentGridPosition() const
{
    return FVector2D(
        FMath::RoundToInt(GetActorLocation().X / TileSize),
        FMath::RoundToInt(GetActorLocation().Y / TileSize)
    );
}
```

### Grid Class Responsibilities

- Knows the state of every tile via `TArray<TArray<ETileContent>> Data`
- Tracks spawned actors per tile via `TArray<TArray<AActor*>> ActorMap`
- Tracks what upgrade is hidden under each soft block via `TArray<TArray<TSubclassOf<AActor>>> UpgradeMap`
- Handles spawning/destroying actors on tiles
- Provides tile queries: `IsTileWalkable()`, `IsTileSoft()`, `GetTileWorldPosition()`, `IsInBounds()`
- Runs a flood-fill (`FloodFill()`) to guarantee door reachability before placing it
- Manages enemy tile reservations to prevent two enemies targeting the same tile

### Grid Config (all configurable in editor)

```cpp
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

UPROPERTY(EditAnywhere, Category = "Grid Config")
int32 PlayerSafeZone = 2;
```

### Grid Growth

Grid size is calculated from stage number on `BeginPlay` (not saved):

```cpp
int32 Growths = (GI->CurrentStage - 1) / GridGrowthPerStages;
BaseGridWidth  = FMath::Min(BaseGridWidth  + Growths * 2, MaxGridWidth);
BaseGridHeight = FMath::Min(BaseGridHeight + Growths * 2, MaxGridHeight);
```

### Tile Reservation System

Enemies reserve their target tile before moving to it. This prevents two enemies from simultaneously targeting the same tile. Reservation is released on death or when the enemy picks a new direction.

```cpp
void ReserveTile(int32 X, int32 Y);
void ReleaseTile(int32 X, int32 Y);
bool IsTileReserved(int32 X, int32 Y) const;
```

Reservation state is stored as `TSet<FIntPoint> ReservedTiles` on the grid.

<br><br>

## 4. Stage Generation

Stages are procedurally generated per stage load. Hard walls never move, their layout is fixed.

### Hard Wall Layout

Border tiles + checkerboard pillars at every even (X, Y) coordinate. Placed in `BeginPlay`.

### What Stays the Same Every Stage

- Hard wall positions
- Player spawn (top-left interior tile: `(1, 1)`)

### What Is Randomized

- Soft block positions (skips player safe zone and existing hard walls)
- Upgrade positions (hidden under random soft blocks based on `UpgradeDensity`)
- Door position (hidden under a random soft block, flood-fill checked for reachability)
- Enemy spawn positions (away from player spawn)

### Stage Generation Flow

1. `BeginPlay`: place hard walls, place top blocks
2. `GenerateGrid()` (called by GameMode): generate soft blocks, place door (skipped for bonus stages), place upgrades
3. GameMode spawns enemies via `SpawnEnemies()` (0.1s delayed to let grid settle)

### Door Placement

The door is hidden under a randomly selected soft block. Before committing to a position, `IsDoorReachable()` runs a BFS flood-fill from player spawn `(1,1)` and verifies that at least one tile adjacent to the door candidate is reachable. This guarantees the player can always reach the door.

### Configurable Per Stage via Data Table (`DT_StageConfig`)

- Enemy types and counts (`TArray<FBombermanStageEnemyEntry>`)
- Stage timer duration
- Soft block density (0.0 - 1.0)
- Upgrade density (0.0 - 1.0)
- Background music
- Door enter sound
- Bonus stage flag

<br><br>

## 5. Bomb System

### Placement

Snaps to nearest grid tile. Collision with the placing player is disabled until the player moves at least 65% of a tile away (`CollisionEnableDistance = 0.65f`). If the player has `BombPass`, collision is never re-enabled for them.

Special case: if the player has `WallPass` and places a bomb on a soft block, the soft block is destroyed first, then the bomb is placed.

### Explosion

Cross-shaped, expanding in 4 directions. Each direction expands up to `BlastRadius` tiles. Rules per tile:

- `HardBlock`: stop, no damage
- `SoftBlock`: destroy it via `DestroyActorOnTile()`, damage actors on that tile, stop
- `Bomb`: chain reaction - detonate that bomb immediately, stop
- `Empty`: damage actors via `BoxOverlapActors` on that tile, continue

Damage detection uses a `BoxOverlapActors` call sized at 90% of the tile to avoid edge bleed. Only `ECC_Pawn` objects are checked.

### Bomb States

```
Placed -> Armed -> Detonating -> Explosion -> Cleanup
```

- `Placed`: bomb spawned, collision disabled for owner
- `Armed`: player stepped off tile, collision enabled (unless BombPass)
- `Detonating`: fuse expired or chain-triggered
- `Explosion`: blast logic runs
- `Cleanup`: Niagara stopped, actor destroyed

### Chain Reactions

When an explosion reaches a `Bomb` tile, the grid tile is cleared immediately (prevents feedback loops) then `TriggerChainReaction()` iterates `TActorIterator<ABombermanBomb>` to find and detonate the bomb actor at that tile. Double-trigger is prevented by the state guard at the start of `Detonate()`.

### Configurable

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Bomb")
float FuseTimer = 2.5f;

UPROPERTY(EditDefaultsOnly, Category = "Bomb")
int32 BlastRadius = 1;

UPROPERTY(EditDefaultsOnly, Category = "Bomb")
float ExplosionSoundCooldown = 0.1f; // deduplicates sound on chain reactions

UPROPERTY(EditDefaultsOnly, Category = "Bomb")
float CollisionEnableDistance = 0.65f;
```

<br><br>

## 6. Player

Inherits from `ACharacter`. Movement is handled by UE's built-in `CharacterMovementComponent`.

### Input

Enhanced Input System (UE5). Three actions:

- `MoveAction` - 2D axis, moves on world forward/right vectors
- `PlaceBombAction` - started trigger
- `DetonateBombAction` - started trigger, only works with `RemoteControl` upgrade

Mapping context is registered in `ABombermanPlayerController::BeginPlay()`. The pause action has `bTriggerWhenPaused = true` set in code to work around a known UE Enhanced Input issue.

### Camera

Fixed spring arm attached to the character, no collision test, rotation locked.

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Camera")
float BaseFOV = 90.f;

UPROPERTY(EditDefaultsOnly, Category = "Camera")
float FOVInterpSpeed = 4.f;

UPROPERTY(EditDefaultsOnly, Category = "Camera")
float FovUpAmount = 10.f; // degrees per FovUp stack
```

Spring arm is set at `-65` degrees pitch, 400 unit arm length. `FovUp` upgrade interpolates the FOV via `FInterpTo` in Tick.

### State (tracked in `UBombermanPlayerState`)

```cpp
int32 Lives = 3;
FBombermanPlayerUpgrades Upgrades;

int32 GetBombCount()    const { return 1 + Upgrades.BombUp; }
int32 GetBlastRadius()  const { return 1 + Upgrades.FireUp; }
int32 GetCurrentScore() const { return FMath::RoundToInt(GetScore()); }
```

Score uses UE's built-in `APlayerState::SetScore()` / `GetScore()` (float internally).

### Upgrades Struct

```cpp
USTRUCT(BlueprintType)
struct FBombermanPlayerUpgrades
{
    int32 BombUp = 0;             // 0-10, persists between stages
    int32 FireUp = 0;             // 0-10, persists between stages
    int32 SpeedUp = 0;            // 0-3, reset on death
    bool  bRemoteControl = false; // reset on death
    bool  bWallPass = false;      // reset on death
    bool  bBombPass = false;      // reset on death
    bool  bFlamePass = false;     // reset on death
    bool  bInvincible = false;    // reset on death
    int32 FovUp = 0;              // 0-5, reset on death
};
```

**On death:** SpeedUp, RemoteControl, WallPass, BombPass, FlamePass, Invincible, and FovUp are all reset. BombUp and FireUp are kept.

### Active Bomb Tracking

`ABombermanCharacter` tracks `TArray<ABombermanBomb*> ActiveBombs` and `int32 ActiveBombCount`. When a bomb is placed, `OnDestroyed` is bound so the count decrements automatically. `DetonateBomb()` detonates `ActiveBombs[0]` (oldest placed bomb).

### WallPass Collision

```cpp
void ABombermanCharacter::SetWallPass(bool bEnabled)
{
    ECollisionResponse Response = bEnabled ? ECR_Ignore : ECR_Block;
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_SoftBlock, Response);
    GetCapsuleComponent()->UpdateOverlaps();
}
```

### Death Flow

1. `HealthComponent->OnDeath` fires
2. Movement stopped, input disabled
3. `DeathAnimDuration` timer fires
4. Lives decremented, boolean upgrades stripped
5. If lives > 0: save state to GameInstance, reload level
6. If lives <= 0: `GameMode->OnGameOver()`

Special case: if the player dies on a bonus stage, `GI->CurrentStage` is incremented before reload so they skip to the next real stage.

<br><br>

## 7. Enemy AI

Each enemy type is a C++ class inheriting from `AEnemyBase`. Movement is tile-to-tile (not per-frame steering). The enemy commits to a target tile, moves there, then decides the next move in `OnTileReached()`.

### Movement Architecture

- `StartMovingToNextTile()`: computes the next tile, reserves it on the grid, sets `TargetWorldPos`
- `Tick()`: interpolates the enemy toward `TargetWorldPos` using `AddMovementInput`. Snaps to tile at <= 2 UU distance
- `OnTileReached()`: virtual, called when arriving at a tile. Base implementation picks a new random direction if current is blocked
- `IsDirectionBlocked()`: virtual, checks bounds, HardBlock, Bomb, SoftBlock (unless `bCanPassThroughSoftBlocks`), and tile reservation
- `PickRandomUnblockedDirection()`: shuffles 4 directions, returns first unblocked. Returns `ZeroVector` if all blocked

### Enemy Table

| Enemy   | AI Approach                                          | Speed | Points | WallPass | Status  |
|---------|------------------------------------------------------|-------|--------|----------|---------|
| Ballom  | Pure random, changes on wall hit                     | 100   | 100    | No       | Done    |
| Onil    | Random + pursue player within `PursuitRange` tiles   | 150   | 200    | No       | Done    |
| Dahl    | Alternates horizontal/vertical axis, random fallback | 125   | 400    | No       | Done    |
| Minvo   | Pursue within range, 25% stubborn retry on blocked   | 115   | 800    | No       | Done    |
| Ovape   | 10% chase chance per tile, ignores soft blocks       | 125   | 2000   | Yes      | Done    |
| Pass    | Always chases, random fallback when blocked          | 175   | 4000   | No       | Done    |
| Pontant | Always chases, tries all 4 directions including back | 200   | 8000   | Yes      | Done    |
| Doria   | Chase + bomb avoidance via Behavior Tree             | TBD   | TBD    | No       | Signed off |

### Shared Base Systems (`AEnemyBase`)

- `UBombermanHealthComponent` - same component as player, `OnDeath` notifies GameMode
- On death: movement disabled, `DeathAnimDuration` timer fires before `Destroy()`
- On capsule overlap with player: calls `TakeDamage(1.f)` on player's health component
- `bCanPassThroughSoftBlocks`: if true, sets `ECC_SoftBlock` response to `ECR_Ignore` in `BeginPlay`
- Debug: `UArrowComponent` showing movement direction, visible in editor, hidden in packaged builds

### Ovape Override

Ovape overrides `IsDirectionBlocked()` to exclude the SoftBlock check (it can pass through soft blocks). It still blocks on HardBlock and Bomb.

<br><br>

## 8. Game Flow

### Class Responsibilities

- `ABombermanGameMode`: master flow controller. Initializes grid, spawns enemies, manages stage timer, handles win/lose, coordinates UI
- `ABombermanGameState`: shared read-only state for UI (`StageState`, `StageTimeRemaining`, `CurrentStage`, `EnemiesRemaining`)
- `UBombermanGameInstance`: persists across level loads (`CurrentStage`, `Lives`, `Score`, `Upgrades`)
- `ABombermanPlayerState`: per-player runtime state (`Lives`, `Upgrades`, `Score`)
- `ABombermanPlayerController`: registers input context, creates HUD widget

### Stage State Enum

```cpp
UENUM(BlueprintType)
enum class EStageState : uint8
{
    WaitingToStart, InProgress, StageClear, GameOver
};
```

### Stage Flow

1. `BeginPlay`: find grid, get GameState, call `StartStage()`
2. `StartStage()`: read stage config from Data Table, call `Grid->GenerateGrid()`, move player to spawn, schedule `SpawnEnemies()` (0.1s delay), start tick timer (1s) and full timer
3. Enemy spawning: shuffled valid tile list (outside player safe zone), spawn each enemy type by count from Data Table
4. Win condition: `EnemiesRemaining <= 0` -> door becomes active (changes material). Player enters door -> `OnPlayerEnteredDoor()` -> `StageClear()`
5. Lose condition: player lives reach 0 -> `OnGameOver()` -> show GameOver widget
6. Timer expire: if bonus stage, auto-complete. Otherwise spawn `EnemyRushCount` (default: 10) Pontants at random empty tiles
7. Stage clear: show StageClearWidget, save to GameInstance + SaveGame, delay `StageClearDelay` seconds, load next stage

### Stage Config (Data Table row `FBombermanStageConfig`)

```cpp
TArray<FBombermanStageEnemyEntry> Enemies;
float StageTimer = 200.f;
float SoftBlockDensity = 0.65f;
float UpgradeDensity = 0.f;  // calculated automatically
USoundBase* BackgroundMusic;
USoundBase* DoorEnterSound;
bool bBonusStage = false;
```

### Configurable in GameMode

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
float StageTimerDuration = 200.f;

UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
int32 StartingLives = 3;

UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
int32 TotalStages = 50;

UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
int32 EnemyRushCount = 10;

UPROPERTY(EditDefaultsOnly, Category = "Stage Config")
float StageClearDelay = 3.f;
```

<br><br>

## 9. Score System

Score is stored in `APlayerState` (UE built-in float field) and persisted through stages via `UBombermanGameInstance`.

| Event                   | Points                           |
|-------------------------|----------------------------------|
| Enemy kill              | Varies by enemy (100 - 8000)     |
| Soft block destroyed    | 10 points                        |
| Stage clear time bonus  | remaining seconds × 10           |

`AddScore(int32 Points)` in GameMode finds `ABombermanPlayerState` and calls `PS->AddScore()` which wraps `SetScore(GetScore() + Points)`.

Score resets to 0 on game over via `UBombermanGameInstance::ResetToDefaults()`.

<br><br>

## 10. Save System

Two separate save slots using UE5's built-in `USaveGame`.

### `UBombermanSaveGame` (slot: `"BombermanSave"`)

Stores game progress. Loaded on `GameInstance::Init()` and on "Continue" from main menu.

```cpp
int32 CurrentStage = 1;
int32 Lives = 3;
int32 Score = 0;
FBombermanPlayerUpgrades Upgrades;
```

Grid size is NOT saved - calculated from stage number at runtime.

### `UBombermanSaveSettings` (slot: `"BombermanSettings"`)

Stores user preferences. Loaded on `GameInstance::Init()`, applied immediately.

```cpp
float MusicVolume = 1.f;
float SFXVolume = 1.f;
float UIVolume = 1.f;
float AmbienceVolume = 1.f;
bool  bMuteOnFocusLost = false;
int32 ResolutionWidth = 1920;
int32 ResolutionHeight = 1080;
int32 WindowMode = 0;    // 0=Fullscreen, 1=Borderless, 2=Windowed (EWindowMode::Type)
int32 QualityPreset = 3; // 0=Low, 1=Medium, 2=High, 3=Epic
```

Save triggers: stage clear, game over, settings changed.

<br><br>

## 11. Settings System

All settings live in `UBombermanGameInstance` and are applied via `ApplySoundSettings()` / `ApplyVideoSettings()`.

### Audio

Four `USoundClass` assets (`MusicSoundClass`, `SFXSoundClass`, `UISoundClass`, `AmbienceSoundClass`) set via `EditDefaultsOnly`. Volume is applied by mutating `SoundClass->Properties.Volume` directly. Mute on focus lost uses `FApp::SetUnfocusedVolumeMultiplier()`.

### Video

Applied through `UGameUserSettings`:
```cpp
Settings->SetScreenResolution(FIntPoint(ResolutionWidth, ResolutionHeight));
Settings->SetFullscreenMode((EWindowMode::Type)WindowMode);
Settings->SetOverallScalabilityLevel(QualityPreset);
Settings->ApplySettings(false);
Settings->SaveSettings();
```

For non-fullscreen modes, the OS window is also resized directly via `GEngine->GameViewport->GetWindow()->Resize()`.

<br><br>

## 12. Music System

Music is managed by `UBombermanGameInstance` through a persistent `UAudioComponent* MusicComponent`.

- `PlayMusic()`: spawns a new 2D looping sound, kills the previous one
- `FadeToMusic()`: fades out existing music over `MusicFadeDuration` (default: 1s), then starts new track
- `StopMusicImmediate()`: stops and nulls the component
- `OnWorldChanged()`: on transition to main menu level, fades out current music

Background music per stage is configured in the Data Table (`FBombermanStageConfig::BackgroundMusic`). Music changes are triggered in `StartStage()`.

<br><br>

## 13. Upgrades

All upgrades are `ABombermanUpgrade` actors placed by the grid system. They float and rotate in Tick (configurable amplitude/speed). On player overlap they apply their effect and call `Destroy()`.

### Upgrade Types (`EUpgradeType`)

| Type          | Effect                                              | Persists |
|---------------|-----------------------------------------------------|----------|
| BombUp        | Max bombs +1 (cap: 10)                              | Yes      |
| FireUp        | Blast radius +1 (cap: 10)                           | Yes      |
| SpeedUp       | Walk speed += SpeedUpIncrement (cap: 3 stacks)      | No       |
| Invincible    | 30s immunity, uses timer to revert                  | No       |
| WallPass      | Pass through soft blocks (calls `SetWallPass()`)    | No       |
| BombPass      | Pass through own bombs (no collision re-enable)     | No       |
| FlamePass     | Immune to own explosion damage                      | No       |
| RemoteControl | Detonate oldest bomb with dedicated input           | No       |
| FovUp         | Camera FOV +FovUpAmount degrees (cap: 5 stacks)     | No       |
| TimeUp        | Adds 20s to stage timer                             | No       |

Upgrades are hidden under soft blocks at stage generation. When a soft block is destroyed, `DestroyActorOnTile()` checks `UpgradeMap[X][Y]` and spawns the upgrade actor if one was assigned there.

<br><br>

## 14. Door System

`ABombermanDoor` is hidden under a soft block at stage start. When the soft block is destroyed, the door actor becomes visible and active.

The door has three material states (shown via `UMaterialBillboardComponent`):
- `DefaultMaterial`: fallback
- `ClosedMaterial`: enemies still alive
- `OpenMaterial`: all enemies dead, door is enterable

`ChangeDoorColor()` is called by GameMode on stage start (after 0.2s delay to let enemies register) and after every enemy death.

On player overlap: if `IsStageCompletable()` is true, player gets invincibility, nearby ambient sound stops, enter VFX and sound play, then `GameMode->OnPlayerEnteredDoor()` is called.

<br><br>

## 15. Anti-Cheat

`FBombermanAntiCheat` is a static-only class called from `ABombermanGameMode::Tick()`. Only runs in packaged builds (`#if !WITH_EDITOR`). Checks run on a randomized interval (3-5 seconds) to reduce fingerprinting.

### Checks

1. `IsDebuggerAttached()` - `IsDebuggerPresent()` WinAPI
2. `IsRemoteDebuggerAttached()` - `CheckRemoteDebuggerPresent()` WinAPI
3. `IsKernelDebuggerAttached()` - `NtQueryInformationProcess` with `ProcessDebugPort (7)` via `ntdll.dll`
4. `DetectAnalysis()` - scans visible window titles for known tool names (Cheat Engine, x64dbg, IDA, Ghidra, OllyDbg), scans window class names for Delphi form patterns (`TMainForm`), scans currently running kernel drivers for suspicious names

### Response

On detection: logs which check fired, shows a `MessageBoxA` identifying the detection reason, calls `FPlatformMisc::RequestExit(true)`.

### XOR Value Template

`TXorValue<T>` in `BombermanXorValue.h` provides XOR obfuscation for sensitive values. Encrypts/decrypts with a random per-instance key using a byte-level XOR over the value's memory. Intended for health, score, and lives fields. Currently not wired up to gameplay yet.

### Planned (not yet implemented)

- Timing-based detection
- `skCrypt`-style compile-time string encryption
- Save file encryption

<br><br>

## 16. Discord Rich Presence

`FBombermanDiscordManager` wraps the Discord Game SDK. Initialized in `GameInstance::OnStart()` with the hardcoded app ID. Callbacks are pumped every frame in `GameMode::Tick()`.

`UpdatePresence()` sets:
- `details`: "Stage X - Score: Y" or "In Main Menu" or "Bonus Stage - Score: Y" (stage -1 triggers bonus display)
- `state`: "N Lives Remaining"
- `largeImageKey`: `"game_logo"`

Called with a 0.2s delay after stage start to let `PlayerState` initialize.

<br><br>

## 17. UI

All UI is built in UMG, created from C++ where needed.

### Screens

| Screen              | Trigger                                    |
|---------------------|--------------------------------------------|
| Main Menu           | Level load                                 |
| HUD                 | Created by `ABombermanPlayerController`    |
| Pause Menu          | Pause input action (`bTriggerWhenPaused`)  |
| Game Over           | `ABombermanGameMode::OnGameOver()`         |
| Stage Clear         | `ABombermanGameMode::StageClear()`         |
| Loading Screen      | Stage transition                           |
| Credits             | Main menu button                           |
| Settings            | Main menu / pause menu                     |

### Component System

Reusable widgets for consistent styling:

- `WBP_Button` - has a `Variant` property: `Primary`, `Secondary`, `Destructive`. Reads styles from `DA_UIColors` data asset automatically. Do NOT hardcode colors inside widgets
- `WBP_BlurryBackground`
- `WBP_GameTip` - shown in main menu and pause menu with random tips
- `WBP_RandomImage`
- `WBP_TransparentBackground`

**Rule:** all colors come from `DA_UIColors`. Editing it updates every widget automatically.

<br><br>

## 18. Debug System

`ABombermanGameMode` has `bShowDebugInfo` (on by default in non-shipping builds via compile-time default).

When enabled, three persistent on-screen messages update every frame:

| Key | Color  | Content                                         |
|-----|--------|-------------------------------------------------|
| 0   | Yellow | Stage number, stage state (int), timer, enemies |
| 1   | Cyan   | Lives, score                                    |
| 2   | Green  | BombUp, FireUp, SpeedUp, Invincible, WallPass   |

Grid has `bDrawDebug` bool that draws color-coded boxes over each tile (Red=Hard, Green=Soft, Yellow=Bomb, Blue=Door, White=Empty).

Console command: `SetStage <N>` (via `UFUNCTION(Exec)` on GameInstance) lets you jump to any stage number during play.

<br><br>

## 19. Multiplayer Considerations

Currently singleplayer only. Architecture is deliberately multiplayer-friendly:

- Player data routes through `PlayerState` / `GameState`, no global singletons
- `GameInstance` persists between levels but player-specific data is synced to `PlayerState` on BeginPlay
- Enemy tile reservation system works per-enemy, not per-player index
- Hardcoded player index `0` exists in enemy AI (`UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)`) - needs replacement for multiplayer
- `ABombermanBomb::LastExplosionSoundTime` is a static, will need to become per-player

Phase roadmap:
- **Phase 1 (current):** Singleplayer
- **Phase 2 (future):** Local co-op/versus (local multiplayer confirmed added per TODO)
- **Phase 3 (if time):** Online via UE5 replication (no dedicated server, P2P)

<br><br>

## 20. Development Schedule

See `TODO.md` for detailed task breakdown and current status.