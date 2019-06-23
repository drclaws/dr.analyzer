#include "stdafx.h"
#include "hook_funcs.h"

#include <detours.h>

#include "hook.h"
#include "flags.h"
#include "Gatherer.h"

#include <iostream>
#include <stdio.h>

pCreateFile2  OrigCreateFile2 = NULL;
pCreateFileA  OrigCreateFileA = NULL;
pCreateFileW  OrigCreateFileW = NULL;
pOpenFileById OrigOpenFileById = NULL;

HANDLE WINAPI NewCreateFile2(
	LPCWSTR                           lpFileName,
	DWORD                             dwDesiredAccess,
	DWORD                             dwShareMode,
	DWORD                             dwCreationDisposition,
	LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams
) {
	HANDLE fileHandle = OrigCreateFile2(
							lpFileName,
							dwDesiredAccess, 
							dwShareMode,
							dwCreationDisposition,
							pCreateExParams);

	if (fileHandle != NULL) {
		GatherFileInfo(fileHandle, GatherFuncType::GatherCreateFile2);
	}

	return fileHandle;
}

HANDLE WINAPI NewCreateFileA(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
) {
	HANDLE fileHandle = OrigCreateFileA(
							lpFileName,
							dwDesiredAccess,
							dwShareMode, 
							lpSecurityAttributes, 
							dwCreationDisposition, 
							dwFlagsAndAttributes,
							hTemplateFile);

	if (fileHandle != NULL) {
		GatherFileInfo(fileHandle, GatherFuncType::GatherCreateFileA);
	}

	return fileHandle;
}

HANDLE WINAPI NewCreateFileW(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
) {
	HANDLE fileHandle = OrigCreateFileW(
							lpFileName,
							dwDesiredAccess,
							dwShareMode,
							lpSecurityAttributes,
							dwCreationDisposition,
							dwFlagsAndAttributes,
							hTemplateFile);

	if (fileHandle != NULL) {
		GatherFileInfo(fileHandle, GatherFuncType::GatherCreateFileW);
	}

	return fileHandle;
}

HANDLE WINAPI NewOpenFileById(
	HANDLE                hVolumeHint,
	LPFILE_ID_DESCRIPTOR  lpFileId,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwFlagsAndAttributes
) {
	HANDLE fileHandle = OrigOpenFileById(
							hVolumeHint,
							lpFileId,
							dwDesiredAccess,
							dwShareMode,
							lpSecurityAttributes,
							dwFlagsAndAttributes);

	if (fileHandle != NULL) {
		GatherFileInfo(fileHandle, GatherFuncType::GatherOpenFileById);
	}

	return fileHandle;
}



pLoadLibraryA OrigLoadLibraryA = NULL;
pLoadLibraryW OrigLoadLibraryW = NULL;
pLoadLibraryExA OrigLoadLibraryExA = NULL;
pLoadLibraryExW OrigLoadLibraryExW = NULL;

HMODULE WINAPI NewLoadLibraryA(
	LPCSTR lpLibFileName
) {
	HMODULE libHmodule = OrigLoadLibraryA(lpLibFileName);

	if (libHmodule != NULL) {
		GatherLibraryInfo(libHmodule, GatherFuncType::GatherLoadLibraryA);
	}

	return libHmodule;
}

HMODULE WINAPI NewLoadLibraryW(
	LPCWSTR lpLibFileName
) {
	HMODULE libHmodule = OrigLoadLibraryW(lpLibFileName);

	if (libHmodule != NULL) {
		GatherLibraryInfo(libHmodule, GatherFuncType::GatherLoadLibraryW);
	}

	return libHmodule;
}

HMODULE WINAPI NewLoadLibraryExA(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD  dwFlags
) {
	HMODULE libHmodule = OrigLoadLibraryExA(
							lpLibFileName,
							hFile,
							dwFlags);

	if (libHmodule != NULL) {
		GatherLibraryInfo(libHmodule, GatherFuncType::GatherLoadLibraryExA);
	}

	return libHmodule;
}

HMODULE WINAPI NewLoadLibraryExW(
	LPCWSTR lpLibFileName,
	HANDLE  hFile,
	DWORD   dwFlags
) {
	HMODULE libHmodule = OrigLoadLibraryExW(
							lpLibFileName,
							hFile,
							dwFlags);

	if (libHmodule != NULL) {
		GatherLibraryInfo(libHmodule, GatherFuncType::GatherLoadLibraryExW);
	}

	return libHmodule;
}



pExitProcess OrigExitProcess = NULL;

void WINAPI NewExitProcess(UINT uExitCode) {
	if (waiterThread != NULL) {
		TerminateThread(waiterThread, 0);
		CloseHandle(waiterThread);
		waiterThread = NULL;
	}

	if (waiterSemaphore != NULL) {
		CloseHandle(waiterSemaphore);
		waiterSemaphore = NULL;
	}

	gatherer->SetDisconnect();

	if (gatherThread != NULL) {
		WaitForSingleObject(gatherThread, INFINITE);
		CloseHandle(gatherThread);
		gatherThread = NULL;
	}

	OrigExitProcess(uExitCode);
}



BOOL GetOrigAddresses()
{
	if ((OrigCreateFile2 = (pCreateFile2)DetourFindFunction("kernel32.dll", "CreateFile2")) == NULL) {
		return FALSE;
	}
	if ((OrigCreateFileA = (pCreateFileA)DetourFindFunction("kernel32.dll", "CreateFileA")) == NULL) {
		return FALSE;
	}
	if ((OrigCreateFileW = (pCreateFileW)DetourFindFunction("kernel32.dll", "CreateFileW")) == NULL) {
		return FALSE;
	}
	if ((OrigOpenFileById = (pOpenFileById)DetourFindFunction("kernel32.dll", "OpenFileById")) == NULL) {
		return FALSE;
	}

	if ((OrigLoadLibraryA = (pLoadLibraryA)DetourFindFunction("kernel32.dll", "LoadLibraryA")) == NULL) {
		return FALSE;
	}
	if ((OrigLoadLibraryW = (pLoadLibraryW)DetourFindFunction("kernel32.dll", "LoadLibraryW")) == NULL) {
		return FALSE;
	}
	if ((OrigLoadLibraryExA = (pLoadLibraryExA)DetourFindFunction("kernel32.dll", "LoadLibraryExA")) == NULL) {
		return FALSE;
	}
	if ((OrigLoadLibraryExW = (pLoadLibraryExW)DetourFindFunction("kernel32.dll", "LoadLibraryExW")) == NULL) {
		return FALSE;
	}
	
	if ((OrigExitProcess = (pExitProcess)DetourFindFunction("kernel32.dll", "ExitProcess")) == NULL) {
		return FALSE;
	}

	return TRUE;
}
