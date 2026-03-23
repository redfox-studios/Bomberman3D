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

bool FBombermanAntiCheat::IsRemoteDebuggerAttached()
{
#if PLATFORM_WINDOWS
	BOOL bRemoteDebugger = 0;
	CheckRemoteDebuggerPresent(GetCurrentProcess(), &bRemoteDebugger);
	return bRemoteDebugger != 0;
#else
	return false;
#endif
}

void FBombermanAntiCheat::RunChecks()
{
	if (IsDebuggerAttached() || IsRemoteDebuggerAttached())
	{
		UE_LOG(LogTemp, Error, TEXT("[AntiCheat] Debugger detected"));
		FPlatformMisc::RequestExit(true);
	}
}
