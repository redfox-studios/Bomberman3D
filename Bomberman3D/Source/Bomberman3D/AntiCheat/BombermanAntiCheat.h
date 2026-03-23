#pragma once

#include "CoreMinimal.h"

class BOMBERMAN3D_API FBombermanAntiCheat
{
  public:
	static bool IsDebuggerAttached();
	static void RunChecks();
};
