# Bomberman3D - TODO

## Phase 1 - Core Systems

### Grid
- [x] ETileContent enum
- [x] Grid data structure (TArray 2D)
- [x] IsTileWalkable()
- [x] IsTileSoft()
- [x] GetTileWorldPosition()
- [x] GetTileContent() - OOB returns HardBlock so explosions stop at map edge
- [x] SetTileContent()
- [x] SpawnActorOnTile()
- [x] DestroyActorOnTile() - also reveals door if it was hidden under the destroyed soft block
- [x] Hard wall layout (border + checkerboard pillars, placed on BeginPlay)
- [x] Procedural soft block generation (density configurable, safe zone around player spawn)
- [x] Door placement (hidden under random soft block, flood-fill check guarantees reachability)
- [x] Debug visualization (color coded: red=hard, green=soft, yellow=bomb, blue=door, white=empty)

### Player
- [x] BombermanCharacter class (inherits ACharacter)
- [x] Top-down movement (4-directional, smooth)
- [x] Fixed camera setup (spring arm, locked rotation, -65 degrees)
- [x] Bomb placement input
- [x] GetCurrentGridPosition() (world pos -> grid coords)
- [x] BombermanPlayerState (lives, upgrades struct)
- [x] FBombermanPlayerUpgrades struct
- [x] BombCount and BlastRadius derived from upgrades (GetBombCount(), GetBlastRadius())
- [x] Health component (BombermanHealthComponent, reusable on enemies)
- [x] Death handling (lose life, respawn or game over)
- [x] Invincibility frames after respawn
- [x] Upgrade reset on death (boolean upgrades stripped, BombUp/FireUp kept)

### Bomb
- [x] ABomb actor class
- [x] Bomb states: Placed -> Armed -> Detonating -> Explosion -> Cleanup
- [x] Fuse timer
- [x] Explosion logic (cross-shaped, 4 directions, stops on hard/soft walls)
- [x] Soft block destruction on explosion (calls DestroyActorOnTile)
- [x] Chain reaction (bomb hit by explosion detonates immediately, double-trigger protected)
- [x] Snap placement to grid tile
- [x] Blast radius from PlayerState (FireUp upgrade)
- [x] Bomb count limit from PlayerState (BombUp upgrade)
- [x] Player/enemy damage on explosion (BoxOverlap on each blast tile)
- [x] Bomb passthrough until player leaves tile
- [x] BombPass fix (never enable collision when player has BombPass)
- [x] WallPass bomb placement fix (destroy soft block before placing bomb)

### Stage
- [x] BombermanGameMode class
- [x] BombermanGameState class (StageState, StageTimeRemaining, CurrentStage, EnemiesRemaining)
- [x] Player spawn at grid spawn position (top-left interior tile)
- [x] Stage timer (200s countdown, ticks every second)
- [x] Enemy spawning (procedural, away from player spawn, configurable count)
- [x] Win condition (all enemies dead -> door active -> player enters door)
- [x] Lose condition (game over screen with restart button)
- [x] Level reload on stage clear
- [x] Stage timer expire -> Enemy Rush (spawn Pontants, count configurable)
- [x] Score system
- [x] Loading screen on stage transition

### Enemies
- [x] AEnemyBase class (health component, death notifies GameMode, configurable speed)
- [x] Enemy kills player on touch
- [x] Tile-to-tile movement (no more per-frame re-evaluation, no corner rounding jank)
- [x] Tile reservation system (prevents two enemies targeting same tile)
- [x] Ballom (random direction, changes on wall hit)

### HUD
- [x] Lives display
- [x] Timer display
- [x] Stage display
- [x] Score display

---

## Phase 2 - Content

### Stage Generation
- [x] Procedural soft block placement (skip player spawn zone)
- [x] Door hidden under random soft block
- [x] Flood-fill check (guarantee door is always reachable)
- [x] Upgrades hidden under random soft blocks (all upgrade types)
- [x] Data Table setup (multiple enemy types + counts, timer, soft block density, upgrade density per stage)
- [x] Grid growth (every X stages, grid grows by 1 in each direction)

### Upgrades & Items
- [x] BombUp (max bombs +1, persists between stages)
- [x] FireUp (blast radius +1, persists between stages)
- [x] SpeedUp (movement speed +1, persists between stages)
  - [ ] Only Available at Stage 4
- [x] RemoteControl (detonate with B button, persists between stages)
- [x] WallPass (pass through soft blocks, persists between stages)
- [x] BombPass (pass through bombs, persists between stages)
- [x] FlamePass (immune to own explosions, persists between stages)
- [x] Invincible (30s immunity to blasts and enemies, persists between stages)
- [ ] Special Items system (challenge-based, available on specific stages only)
  - [ ] B Panel (reach exit without killing any enemies,                                    stages 6 8 14 16 22 24 30 32 38 40 45 48)   - 10 000 pts
  - [ ] Goddess (defeat all enemies then walk outer ring,                                   stages 1 7 9 15 17 23 25 31 33 39 41 47 49) - 20 000 pts
  - [ ] Cola (defeat all enemies, find exit, keep walking 15s without releasing D-Pad,      stages 4 12 20 28 36 44)                    - 30 000 pts
  - [ ] Famicon (defeat all enemies, detonate 248 chain reaction,                           stages 3 11 19 27 35 43)                    - 500 000 pts
  - [ ] Programmer Nakamoto (defeat all enemies without destroying any soft block,          stages 2 10 18 26 34 42 50)                 - 10 000 000 pts
  - [ ] Dezeniman (destroy all soft blocks without killing enemies, then bomb exit 3 times, stages 5 13 21 29 37 45)                    - 20 000 000 pts

### Save System
- [x] GameInstance persists stage, lives, upgrades across level loads
- [x] UBombermanSaveGame class (persist between game sessions)
- [x] Save on stage clear + game over
- [x] Load on game start / continue

### Stage Progression
- [x] Load next stage on clear
- [x] Enemy Rush (Pontants on timer expire, count configurable)
- [x] All upgrades persist between stages
- [x] 50 total stages in data table
  - [x] Win after 50 stages

---

## Phase 3 - AI

- [x] Onil (random + pursue if player nearby)
- [x] Dahl (alternates axis)
- [x] Minvo (random + pursue, can get stuck)
- [x] Ovape (mostly ignores player)
- [x] Pass (aggressive chase)
- [x] Pontant (always chase, fastest)
- [ ] Doria (chase + bomb avoidance) <- signed off for now, revisit if time allows
- [x] Tile occupancy (max one enemy per tile)
- [x] Tile reservation system (prevents movement conflicts)

### VFX & Sound
- [x] Explosion VFX support
- [ ] Soft block destroy VFX (probably not gonna do this)
- [x] Upgrade pickup VFX support
- [x] Background music
  - [x] Background music in game
  - [x] Background music in main menu
  - [x] Background music fade when switching between different ones
- [x] SFX support (bomb place, explosion, pickup, death, enter door)
  - [ ] Nearby door sound
- [ ] Actual VFX/SFX assets
- [x] Entering portal in the final stage should have a different sound

---

## Phase 4 - Polish

### Code Quality
- [x] .clang-format (Allman braces, tabs, pointer left, no comment reflow)
- [x] .editorconfig (utf-8, LF, tabs)
- [x] fix-encoding.ps1 (UTF-16 -> UTF-8 batch converter)

### UI
- [x] Main Menu (New Game, Continue, Quit)
- [x] Pause Menu (Resume, Save and Quit, Save and go to Main Menu)
- [x] Game Over screen
- [x] Stage Clear screen
- [x] Loading Screen
- [x] Credits Screen
- [ ] Settings (keybind remapping via Enhanced Input)
- [x] Random game tips in main menu and pause menu

### Music (code, not assets)
- [x] Background music persistent trough levels
- [x] Background music config in Data Table

### Stages
- [x] Bonus stages
  - [x] FROM GDD: every 5 levels a bonus stage appears, on said bonus stage only Balloms spawn and no soft blocks, this stage is only 30 seconds long and there are no doors in this stage, after the timer runs out the player goes to the next stage, the music changes in this level to be more fast paced
  - [x] They are gonna be appearing as normal stages in the Data Table, they will just have BONUS as row name, nothing in the code, the enemy spawn, music, etc will be configured by the game designer in data table

### Final
- [ ] Full bugfix pass
- [x] Discord SDK
- [ ] Local co-op/versus (if time allows)
  - [ ] Full Multiplayer bugfix pass
- [ ] Optimization - currently the game build has ~300mb which is pretty small (and good) for UE game but im pretty sure we can lower this

---

## Known Risks
- Doria AI complexity -> signed off for now, revisit if time allows
- Procedural gen edge cases -> flood-fill check is in, keep an eye on edge cases
- Multiplayer refactor cost -> no global player singletons, always use PlayerState/GameState
- Special items with complex conditions (248 chain reaction, outer ring walk) -> design carefully, may need dedicated tracking components

## Known Bugs & Issues
- enemy can walk through my bomb when i place the bomb on the enemy (makes sense since we have bombpass upgrade and the temporary bombpass for handling collision with player after bomb placement)

### Probably Fixed
- player can get stuck in corners
  - player can be invincible after stucking themselves in the corner and can also noclip into the enemy... idk how but my teacher managed to discover this bug...

## Security
- [ ] my cheat for the game
- [ ] Anticheat
  - [ ] XOR encrypt important values (health, score...)
  - [ ] anti debug
  - [ ] skCrypt for strings
  - [ ] save file encryption/decryption - XOR first, AES maybe later

now yes, implementing anticheat is basically useless if the game doesnt have multiplayer yet and there isnt any leaderboard with score, or smth. But since im really into low level stuff, i will make the cheat and anticheat anyway (if there will be enough time). Also because i want to laugh at the noobies in my class that installed CE without even knowing how computers work and want to be like me lol

### Multiplayer Bugs
- walking through other players bombs needs all-bomb check (ignored until multiplayer)