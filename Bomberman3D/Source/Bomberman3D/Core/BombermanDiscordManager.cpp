// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Core/BombermanDiscordManager.h"
#include "discord_rpc.h"

void FBombermanDiscordManager::Init(int64 AppId)
{
	DiscordEventHandlers Handlers;
	FMemory::Memzero(&Handlers, sizeof(Handlers));

	Discord_Initialize(TCHAR_TO_ANSI(*FString::Printf(TEXT("%lld"), AppId)), &Handlers, 1, nullptr);
	bInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("[Discord] Initialized"));
}

void FBombermanDiscordManager::Shutdown()
{
	if (!bInitialized) return;
	Discord_Shutdown();
	bInitialized = false;
}

void FBombermanDiscordManager::UpdatePresence(int32 Stage, int32 Lives, bool bInMainMenu)
{
	if (!bInitialized) return;

	DiscordRichPresence Presence;
	FMemory::Memzero(&Presence, sizeof(Presence));

	FString Details;
	FString State;

	if (bInMainMenu)
	{
		Details = TEXT("In Main Menu");
		State = TEXT("Singleplayer");
	}
	else
	{
		Details = FString::Printf(TEXT("Stage %d / 50"), Stage);
		State = FString::Printf(TEXT("%d lives remaining - Singleplayer"), Lives);
	}

	Presence.details = TCHAR_TO_ANSI(*Details);
	Presence.state = TCHAR_TO_ANSI(*State);
	Presence.largeImageKey = "game_logo"; // set this up in Discord dev portal later
	Presence.largeImageText = "Bomberman3D";

	Discord_UpdatePresence(&Presence);
}

void FBombermanDiscordManager::RunCallbacks()
{
	if (!bInitialized) return;
	Discord_RunCallbacks();
}
