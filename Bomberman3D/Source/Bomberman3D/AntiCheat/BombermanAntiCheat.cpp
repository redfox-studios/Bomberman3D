#include "AntiCheat/BombermanAntiCheat.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#pragma comment(lib, "ntdll.lib")
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

bool FBombermanAntiCheat::IsKernelDebuggerAttached()
{
#if PLATFORM_WINDOWS
	typedef NTSTATUS(WINAPI* NtQueryInformationProcessFn)(HANDLE, UINT, PVOID, ULONG, PULONG);

	HMODULE NtDll = GetModuleHandleA("ntdll.dll");
	if (!NtDll) return false;

#pragma warning(push)
#pragma warning(disable : 4191)
	NtQueryInformationProcessFn NtQueryInformationProcess =
		(NtQueryInformationProcessFn)GetProcAddress(NtDll, "NtQueryInformationProcess");
#pragma warning(pop)
	if (!NtQueryInformationProcess) return false;

	HANDLE DebugPort = nullptr;
	NTSTATUS Status = NtQueryInformationProcess(
		GetCurrentProcess(),
		7, // ProcessDebugPort
		&DebugPort,
		sizeof(DebugPort),
		nullptr
	);

	return NT_SUCCESS(Status) && DebugPort != nullptr;
#else
	return false;
#endif
}

// TODO: PROCESS NAME CHECK
// 
// process name checks are bypassable by just renaming the CE executable.
// But for my braindead classmates who installed CE without knowing how computers work,
// this is honestly more than enough.

// ------ checks ------

void FBombermanAntiCheat::RunChecks()
{
	if (IsDebuggerAttached() || IsRemoteDebuggerAttached() || IsKernelDebuggerAttached())
	{
		UE_LOG(LogTemp, Error, TEXT("[AntiCheat] bad boy detected."));
		FPlatformMisc::RequestExit(true);
	}
}
