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

void FBombermanDiscordManager::UpdatePresence(int32 Stage, int32 Lives, int32 Score, bool bInMainMenu)
{
	if (!bInitialized) return;

	DiscordRichPresence Presence;
	FMemory::Memzero(&Presence, sizeof(Presence));

	// static buffers so the pointers stay valid
	static char DetailsBuffer[128];
	static char StateBuffer[128];

	if (bInMainMenu)
	{
		FCStringAnsi::Strcpy(DetailsBuffer, "In Main Menu");
		FCStringAnsi::Strcpy(StateBuffer, "Singleplayer");
	}
	else
	{
		if (Stage < 0)
		FCStringAnsi::Snprintf(DetailsBuffer, sizeof(DetailsBuffer), "Bonus Stage - Score: %d", Score);
	else
		FCStringAnsi::Snprintf(DetailsBuffer, sizeof(DetailsBuffer), "Stage %d - Score: %d", Stage, Score);
		FCStringAnsi::Snprintf(StateBuffer, sizeof(StateBuffer), "%d Lives Remaining", Lives);
	}

	Presence.details = DetailsBuffer;
	Presence.state = StateBuffer;
	Presence.largeImageKey = "game_logo";
	Presence.largeImageText = "Bomberman3D";

	Discord_UpdatePresence(&Presence);
}

void FBombermanDiscordManager::RunCallbacks()
{
	if (!bInitialized) return;
	Discord_RunCallbacks();
}
