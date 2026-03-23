#pragma once

#include "CoreMinimal.h"

class BOMBERMAN3D_API FBombermanAntiCheat
{
  public:
	static bool IsDebuggerAttached();
	static void RunChecks();
	static bool IsRemoteDebuggerAttached();
	static bool IsKernelDebuggerAttached();
	static bool DetectAnalysis();

  private:
	static double LastCheckTime;
	static double NextInterval;
};
