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

bool FBombermanAntiCheat::DetectAnalysis()
{
#if PLATFORM_WINDOWS

	// --- window title check ---
	const char* SuspiciousWindows[] = {
		"Cheat Engine",
		"x64dbg",
		"IDA",
		"Ghidra",
		"OllyDbg"
	};

	for (const char* title : SuspiciousWindows)
	{
		if (FindWindowA(NULL, title))
		{
			return true;
		}
	}

	// --- process name check ---
	const char* SuspiciousProcesses[] = {
		"cheatengine",
		"x64dbg",
		"ida",
		"ghidra",
		"ollydbg"
	};

	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(pe);

		if (Process32First(snap, &pe))
		{
			do
			{
				FString name = UTF8_TO_TCHAR(pe.szExeFile);
				name = name.ToLower();

				for (const char* bad : SuspiciousProcesses)
				{
					if (name.Contains(UTF8_TO_TCHAR(bad)))
					{
						CloseHandle(snap);
						return true;
					}
				}

			} while (Process32Next(snap, &pe));
		}

		CloseHandle(snap);
	}

	// --- module scan (injected dlls etc) ---
	HANDLE modSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (modSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 mod;
		mod.dwSize = sizeof(mod);

		if (Module32First(modSnap, &mod))
		{
			do
			{
				FString modName = UTF8_TO_TCHAR(mod.szModule);
				modName = modName.ToLower();

				if (modName.Contains("cheat") || modName.Contains("dbg"))
				{
					CloseHandle(modSnap);
					return true;
				}

			} while (Module32Next(modSnap, &mod));
		}

		CloseHandle(modSnap);
	}

#endif

	// process name checks are bypassable by just renaming the executable.
	// But for my braindead classmates who installed CE without knowing how computers, nor game engines work,
	// this is honestly more than enough.

	return false;
}

// ------ checks ------

void FBombermanAntiCheat::RunChecks()
{
	if (
		IsDebuggerAttached() ||
		IsRemoteDebuggerAttached() ||
		IsKernelDebuggerAttached() ||
		DetectAnalysis()
	)
	{
		UE_LOG(LogTemp, Error, TEXT("[AntiCheat] caught ya fool"));
		FPlatformMisc::RequestExit(true);
	}
}
