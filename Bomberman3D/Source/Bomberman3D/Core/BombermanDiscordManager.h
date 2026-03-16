// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class BOMBERMAN3D_API FBombermanDiscordManager
{
  public:
	void Init(int64 AppId);
	void Shutdown();
	void UpdatePresence(int32 Stage, int32 Lives, bool bInMainMenu);
	void RunCallbacks();

  private:
	bool bInitialized = false;
};
