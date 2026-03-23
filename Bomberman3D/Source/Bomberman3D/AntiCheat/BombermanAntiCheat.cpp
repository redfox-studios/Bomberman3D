#include "AntiCheat/BombermanAntiCheat.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

bool FBombermanAntiCheat::IsDebuggerAttached()
{
#if PLATFORM_WINDOWS
	return ::IsDebuggerPresent();
#else
	return false;
#endif
}

void FBombermanAntiCheat::RunChecks()
{
	if (IsDebuggerAttached())
	{
		UE_LOG(LogTemp, Error, TEXT("[AntiCheat] Debugger detected. Terminating."));
		FPlatformMisc::RequestExit(true);
	}
}
