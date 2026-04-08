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

// --- yes ---

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

	// ---- window title scan ----
	{
		HWND hwnd = GetTopWindow(NULL);
		while (hwnd)
		{
			// skip invisible and minimized windows
			if (!IsWindowVisible(hwnd) || IsIconic(hwnd))
			{
				hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
				continue;
			}

			char className[256];
			GetClassNameA(hwnd, className, sizeof(className));
			FString cls = UTF8_TO_TCHAR(className);

			// skip explorer, taskbar, etc
			if (cls.Equals(TEXT("CabinetWClass"), ESearchCase::IgnoreCase) ||
				cls.Equals(TEXT("ExploreWClass"), ESearchCase::IgnoreCase) ||
				cls.Equals(TEXT("Shell_TrayWnd"), ESearchCase::IgnoreCase))
			{
				hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
				continue;
			}

			char title[256];
			GetWindowTextA(hwnd, title, sizeof(title));
			FString t = UTF8_TO_TCHAR(title);
			t = t.ToLower();

			if (t.Contains("cheat engine") ||
				t.Contains("cheatengine") ||
				t.Contains("x64dbg") ||
				t.Contains("ida ") || // ida with space to get rid of false positives
				t.Contains("ghidra") ||
				t.Contains("ollydbg"))
			{
				return true;
			}

			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		}
	}

	// ---- class name scan ----
	{
		HWND hwnd = GetTopWindow(NULL);

		while (hwnd)
		{
			char className[256];
			GetClassNameA(hwnd, className, sizeof(className));

			FString c = UTF8_TO_TCHAR(className);
			c = c.ToLower();

			if (c.Contains("tmainform") /* || c.Contains("tapplication") */)
			{
				return true;
			}

			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		}
	}

	// ---- driver scan ----
	{
		SC_HANDLE sc = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);

		if (sc)
		{
			DWORD bytesNeeded = 0, count = 0;

			EnumServicesStatusEx(
				sc,
				SC_ENUM_PROCESS_INFO,
				SERVICE_DRIVER,
				SERVICE_STATE_ALL,
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
					SERVICE_STATE_ALL,
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
						CloseServiceHandle(sc);
						return true;
					}
				}
			}

			CloseServiceHandle(sc);
		}
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
	double Now = FPlatformTime::Seconds();

	if (Now - LastCheckTime < NextInterval)
		return;

	LastCheckTime = Now;
	NextInterval = FMath::FRandRange(3.0, 5.0);

	if (
		IsDebuggerAttached() ||
		IsRemoteDebuggerAttached() ||
		IsKernelDebuggerAttached() ||
		DetectAnalysis()
	)
	{
		UE_LOG(LogTemp, Error, TEXT("[AntiCheat] caught ya fool"));
		// FPlatformMisc::RequestExit(true);
	}
}
