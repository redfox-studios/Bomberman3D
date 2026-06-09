#include "AntiCheat/BombermanAntiCheat.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <winsvc.h>
#pragma comment(lib, "ntdll.lib")
#include "Windows/HideWindowsPlatformTypes.h"
#endif

double FBombermanAntiCheat::LastCheckTime = 0.0;
double FBombermanAntiCheat::NextInterval = 3.0;

// ===================================================================
//  DEBUGGER CHECKS
// ===================================================================

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
	typedef NTSTATUS(WINAPI * NtQueryInformationProcessFn)(HANDLE, UINT, PVOID, ULONG, PULONG);

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

// ===================================================================
//  MAIN DETECTION (now with full logging + fewer false positives)
// ===================================================================

bool FBombermanAntiCheat::DetectAnalysis()
{
#if PLATFORM_WINDOWS

	// ---- window title scan (skip our own game windows) ----
	{
		HWND hwnd = GetTopWindow(NULL);
		while (hwnd)
		{
			DWORD dwProcessId = 0;
			GetWindowThreadProcessId(hwnd, &dwProcessId);
			if (dwProcessId == GetCurrentProcessId()) // skip our own windows
			{
				hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
				continue;
			}

			if (!IsWindowVisible(hwnd) || IsIconic(hwnd))
			{
				hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
				continue;
			}

			char className[256] = {};
			GetClassNameA(hwnd, className, sizeof(className));
			FString cls = UTF8_TO_TCHAR(className);

			// skip explorer/taskbar/etc
			if (cls.Equals(TEXT("CabinetWClass"), ESearchCase::IgnoreCase) ||
				cls.Equals(TEXT("ExploreWClass"), ESearchCase::IgnoreCase) ||
				cls.Equals(TEXT("Shell_TrayWnd"), ESearchCase::IgnoreCase))
			{
				hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
				continue;
			}

			char title[256] = {};
			GetWindowTextA(hwnd, title, sizeof(title));
			FString originalTitle = UTF8_TO_TCHAR(title);
			FString lowerTitle = originalTitle.ToLower();

			if (lowerTitle.Contains("cheat engine") ||
				lowerTitle.Contains("cheatengine") ||
				lowerTitle.Contains("x64dbg") ||
				lowerTitle.Contains("ida ") ||
				lowerTitle.Contains("ghidra") ||
				lowerTitle.Contains("ollydbg"))
			{
				UE_LOG(LogTemp, Error, TEXT("[AntiCheat] SUSPICIOUS WINDOW TITLE: \"%s\" (class: %s)"), *originalTitle, *cls);
				return true;
			}

			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		}
	}

	// ---- class name scan (Delphi/Cheat Engine main form) ----
	{
		HWND hwnd = GetTopWindow(NULL);
		while (hwnd)
		{
			DWORD dwProcessId = 0;
			GetWindowThreadProcessId(hwnd, &dwProcessId);
			if (dwProcessId == GetCurrentProcessId())
			{
				hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
				continue;
			}

			char className[256] = {};
			GetClassNameA(hwnd, className, sizeof(className));
			FString c = UTF8_TO_TCHAR(className);
			c = c.ToLower();

			if (c.Contains("tmainform"))
			{
				UE_LOG(LogTemp, Error, TEXT("[AntiCheat] SUSPICIOUS WINDOW CLASS: \"%s\""), *c);
				return true;
			}

			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		}
	}

	// ---- driver scan (ONLY currently RUNNING drivers -> way fewer false positives) ----
	{
		SC_HANDLE sc = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
		if (sc)
		{
			DWORD bytesNeeded = 0, count = 0;

			// First call to get required buffer size
			EnumServicesStatusEx(
				sc,
				SC_ENUM_PROCESS_INFO,
				SERVICE_DRIVER,
				SERVICE_ACTIVE, // <- CHANGED: only running drivers
				NULL,
				0,
				&bytesNeeded,
				&count,
				NULL,
				NULL
			);

			TArray<BYTE> buffer;
			buffer.SetNum(bytesNeeded);

			if (EnumServicesStatusEx(
					sc,
					SC_ENUM_PROCESS_INFO,
					SERVICE_DRIVER,
					SERVICE_ACTIVE, // <- CHANGED
					buffer.GetData(),
					bytesNeeded,
					&bytesNeeded,
					&count,
					NULL,
					NULL
				))
			{
				auto services = (ENUM_SERVICE_STATUS_PROCESS*)buffer.GetData();

				for (DWORD i = 0; i < count; i++)
				{
					FString name = services[i].lpServiceName;
					name = name.ToLower();

					if (name.Contains("cheat") || name.Contains("cedriver"))
					{
						UE_LOG(LogTemp, Error, TEXT("[AntiCheat] SUSPICIOUS DRIVER RUNNING: \"%s\""), *name);
						CloseServiceHandle(sc);
						return true;
					}
				}
			}

			CloseServiceHandle(sc);
		}
	}

#endif

	return false;
}

// ===================================================================
//  RUN CHECKS (now logs exactly which check fired)
// ===================================================================

void FBombermanAntiCheat::RunChecks() // called in the gamemode
{
	double Now = FPlatformTime::Seconds();

	if (Now - LastCheckTime < NextInterval)
		return;

	LastCheckTime = Now;
	NextInterval = FMath::FRandRange(3.0, 5.0);

	bool bCheatDetected = false;

	if (IsDebuggerAttached())
	{
		UE_LOG(LogTemp, Error, TEXT("[AntiCheat] IsDebuggerAttached() -> TRUE"));
		bCheatDetected = true;
	}
	if (IsRemoteDebuggerAttached())
	{
		UE_LOG(LogTemp, Error, TEXT("[AntiCheat] IsRemoteDebuggerAttached() -> TRUE"));
		bCheatDetected = true;
	}
	if (IsKernelDebuggerAttached())
	{
		UE_LOG(LogTemp, Error, TEXT("[AntiCheat] IsKernelDebuggerAttached() -> TRUE"));
		bCheatDetected = true;
	}
	if (DetectAnalysis()) // logging happens inside
	{
		bCheatDetected = true;
	}

	if (bCheatDetected)
	{
		UE_LOG(LogTemp, Error, TEXT("[AntiCheat] CHEAT DETECTED - check the logs above for the exact reason"));

#if PLATFORM_WINDOWS
		MessageBoxA(NULL, "Cheater Detected!\n\nCheck the Unreal Output Log for details.", "Bomberman Anti-Cheat", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SYSTEMMODAL);
#endif

		FPlatformMisc::RequestExit(true);
	}
}
