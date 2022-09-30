//#include "menu/registrywindow.hpp"
#include <Windows.h>
#include <Windowsx.h>
#include <Psapi.h>
#include "anti_dbg.h"

#include <fstream>
#include <signal.h>
#include <thread>
#include <TlHelp32.h>
#include <Shlwapi.h>

#pragma comment(lib, "psapi.lib")
#pragma once

namespace sec
{
	__forceinline BOOL sec::IsRemoteSession(void)
	{
		return GetSystemMetrics(SM_REMOTESESSION);
	}

	__forceinline BOOL sec::EnablePriv(LPCSTR lpszPriv)
	{
		HANDLE hToken;
		LUID luid;
		TOKEN_PRIVILEGES tkprivs;
		ZeroMemory(&tkprivs, sizeof(tkprivs));

		if (!OpenProcessToken(GetCurrentProcess(), (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), &hToken))
			return FALSE;

		if (!LookupPrivilegeValue(NULL, lpszPriv, &luid)) {
			CloseHandle(hToken); return FALSE;
		}

		tkprivs.PrivilegeCount = 1;
		tkprivs.Privileges[0].Luid = luid;
		tkprivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		BOOL bRet = AdjustTokenPrivileges(hToken, FALSE, &tkprivs, sizeof(tkprivs), NULL, NULL);
		CloseHandle(hToken);
		return bRet;
	}
	__forceinline void sec::shutdown()
	{
		exit(EXIT_FAILURE);
	}

	__forceinline void sec::Session()
	{
		std::this_thread::sleep_for(std::chrono::seconds(240));
		shutdown();
	}

	__forceinline BOOL sec::MakeCritical()
	{
		return 0;
	}

	__forceinline void sec::killProcessByName(const char* filename)
	{
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
		PROCESSENTRY32 pEntry;
		pEntry.dwSize = sizeof(pEntry);
		BOOL hRes = Process32First(hSnapShot, &pEntry);
		while (hRes)
		{
			if (strcmp(pEntry.szExeFile, filename) == 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
					(DWORD)pEntry.th32ProcessID);
				if (hProcess != NULL)
				{
					TerminateProcess(hProcess, 9);
					CloseHandle(hProcess);
				}
			}
			hRes = Process32Next(hSnapShot, &pEntry);
		}
		CloseHandle(hSnapShot);
	}

	__forceinline bool sec::IsDebuggersInstalledStart()
	{
		LPVOID drivers[2048];
		DWORD cbNeeded;
		int cDrivers, i;

		if (EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded) && cbNeeded < sizeof(drivers))
		{
			TCHAR szDriver[2048];

			cDrivers = cbNeeded / sizeof(drivers[0]);

			for (i = 0; i < cDrivers; i++)
			{
				if (GetDeviceDriverBaseName(drivers[i], szDriver, sizeof(szDriver) / sizeof(szDriver[0])))
				{
					std::string strDriver = szDriver;
					if (strDriver.find(_("kprocesshacker")) != std::string::npos)
					{
						return true;
					}
					if (strDriver.find(_("npf")) != std::string::npos)
					{
						return true;
					}
					if (strDriver.find(_("TitanHide")) != std::string::npos)
					{
						return true;
					}
					if (strDriver.find(_("vgk")) != std::string::npos)
					{
						return true;
					}
					if (strDriver.find(_("faceitac")) != std::string::npos)
					{
						return true;
					}
					if (strDriver.find(_("EasyAntiCheat")) != std::string::npos)
					{
						return true;
					}
					if (strDriver.find(_("BEDaisy")) != std::string::npos)
					{
						return true;
					}
					if (strDriver.find(_("SharpOD_Drv")) != std::string::npos)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	__forceinline bool sec::IsDebuggersInstalledThread()
	{
		LPVOID drivers[2048];
		DWORD cbNeeded;
		int cDrivers, i;

		if (EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded) && cbNeeded < sizeof(drivers))
		{
			TCHAR szDriver[2048];

			cDrivers = cbNeeded / sizeof(drivers[0]);

			for (i = 0; i < cDrivers; i++)
			{
				if (GetDeviceDriverBaseName(drivers[i], szDriver, sizeof(szDriver) / sizeof(szDriver[0])))
				{
					std::string strDriver = szDriver;
					if (strDriver.find(_("kprocesshacker")) != std::string::npos)
					{
						return true;
					}
					if (strDriver.find(_("npf")) != std::string::npos)
					{
						return true;
					}
					if (strDriver.find(_("TitanHide")) != std::string::npos)
					{
						return true;
					}
					if (strDriver.find(_("SharpOD_Drv")) != std::string::npos)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	__forceinline DWORD sec::GetProcessIdFromName(LPCTSTR szProcessName)
	{
		PROCESSENTRY32 pe32;
		HANDLE hSnapshot = NULL;
		SecureZeroMemory(&pe32, sizeof(PROCESSENTRY32));

		// We want a snapshot of processes
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		// Check for a valid handle, in this case we need to check for
		// INVALID_HANDLE_VALUE instead of NULL
		if (hSnapshot == INVALID_HANDLE_VALUE)
		{
			return 0;
		}

		// Now we can enumerate the running process, also 
		// we can't forget to set the PROCESSENTRY32.dwSize member
		// otherwise the following functions will fail
		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hSnapshot, &pe32) == FALSE)
		{
			// Cleanup the mess
			CloseHandle(hSnapshot);
			return 0;
		}

		// Do our first comparison
		if (StrCmpI(pe32.szExeFile, szProcessName) == 0)
		{
			// Cleanup the mess
			CloseHandle(hSnapshot);
			return pe32.th32ProcessID;
		}

		// Most likely it won't match on the first try so 
		// we loop through the rest of the entries until
		// we find the matching entry or not one at all
		while (Process32Next(hSnapshot, &pe32))
		{
			if (StrCmpI(pe32.szExeFile, szProcessName) == 0)
			{
				// Cleanup the mess
				CloseHandle(hSnapshot);
				return pe32.th32ProcessID;
			}
		}
		// If we made it this far there wasn't a match, so we'll return 0
		// _tprintf(_T("\n-> Process %s is not running on this system ..."), szProcessName);

		CloseHandle(hSnapshot);
		return 0;
	}

	__forceinline bool sec::analysis()
	{
		std::string szProcesses[] =
		{
			_("HttpAnalyzerStdV5.exe"),
			_("ollydbg.exe"),
			_("x64dbg.exe"),
			_("x32dbg.exe"),
			_("die.exe"),
			_("tcpview.exe"),			// Part of Sysinternals Suite
			_("autoruns.exe"),			// Part of Sysinternals Suite
			("autorunsc.exe"),		// Part of Sysinternals Suite
			_("filemon.exe"),			// Part of Sysinternals Suite
			_("procmon.exe"),			// Part of Sysinternals Suite
			_("regmon.exe"),			// Part of Sysinternals Suite
			_("procexp.exe"),			// Part of Sysinternals Suite
			_("idaq.exe"),				// IDA Pro Interactive Disassembler
			_("idaq64.exe"),			// IDA Pro Interactive Disassembler
			_("ida.exe"),				// IDA Pro Interactive Disassembler
			_("ida64.exe"),			// IDA Pro Interactive Disassembler
			_("ImmunityDebugger.exe"), // ImmunityDebugger
			_("Wireshark.exe"),		// Wireshark packet sniffer
			_("dumpcap.exe"),			// Network traffic dump tool
			_("HookExplorer.exe"),		// Find various types of runtime hooks
			_("ImportREC.exe"),		// Import Reconstructor
			_("PETools.exe"),			// PE Tool
			_("LordPE.exe"),			// LordPE
			_("dumpcap.exe"),			// Network traffic dump tool
			_("SysInspector.exe"),		// ESET SysInspector
			_("proc_analyzer.exe"),	// Part of SysAnalyzer iDefense
			_("sysAnalyzer.exe"),		// Part of SysAnalyzer iDefense
			_("sniff_hit.exe"),		// Part of SysAnalyzer iDefense
			_("windbg.exe"),			// Microsoft WinDbg
			_("joeboxcontrol.exe"),	// Part of Joe Sandbox
			_("joeboxserver.exe"),		// Part of Joe Sandbox
			_("fiddler.exe"),
			_("tv_w32.exe"),
			_("tv_x64.exe"),
			_("Charles.exe"),
			_("netFilterService.exe"),
			_("HTTPAnalyzerStdV7.exe")
		};

		WORD iLength = sizeof(szProcesses) / sizeof(szProcesses[0]);
		for (int i = 0; i < iLength; i++)
		{
			if (GetProcessIdFromName(szProcesses[i].c_str()))
			{
				killProcessByName(szProcesses[i].c_str());
				return true;
			}
		}
		return false;
	}

	__forceinline void sec::clown()
	{
		MakeCritical();
		shutdown();
	}

	__forceinline bool sec::start()
	{
		return IsRemoteSession() || IsDebuggersInstalledThread() || analysis();
	}
	__forceinline void sec::ErasePEHeaderFromMemory()
	{
		DWORD OldProtect = 0;

		// Get base address of module
		char* pBaseAddr = (char*)GetModuleHandle(NULL);

		// Change memory protection
		VirtualProtect(pBaseAddr, 4096, // Assume x86 page size
			PAGE_READWRITE, &OldProtect);

		// Erase the header
		ZeroMemory(pBaseAddr, 4096);
	}
	__forceinline bool HideThread(HANDLE hThread)
	{
		typedef NTSTATUS(NTAPI* pNtSetInformationThread)
			(HANDLE, UINT, PVOID, ULONG);

		NTSTATUS Status;

		// Get NtSetInformationThread
		pNtSetInformationThread NtSIT = (pNtSetInformationThread)
			GetProcAddress(GetModuleHandle(TEXT(_("ntdll.dll"))), _("NtSetInformationThread"));
		// Shouldn't fail
		if (NtSIT == NULL)
			return false;

		// Set the thread info
		if (hThread == NULL)
			Status = NtSIT(GetCurrentThread(),
				0x11, //ThreadHideFromDebugger
				0, 0);
		else
			Status = NtSIT(hThread, 0x11, 0, 0);

		if (Status != 0x00000000)
			return false;
		else
			return true;
	}

	__forceinline void sec::ST()
	{
		HideThread(GetCurrentThread);
		while (true)
		{
			if (start())
			{
				Sleep(5000);
				clown();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
}