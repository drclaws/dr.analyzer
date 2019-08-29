#include "stdafx.h"
#include "hook_funcs.h"

#include <detours.h>

#include "hook.h"
#include "flags.h"
#include "Gatherer.h"
#include "GatherInfo.h"

#include <stdio.h>


typedef HANDLE(WINAPI *pCreateFile2)(
	LPCWSTR                           lpFileName,
	DWORD                             dwDesiredAccess,
	DWORD                             dwShareMode,
	DWORD                             dwCreationDisposition,
	LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams
	);
typedef HANDLE(WINAPI *pCreateFileA)(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
	);
typedef HANDLE(WINAPI *pCreateFileW)(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
	);
typedef HANDLE(WINAPI *pOpenFileById)(
	HANDLE                hVolumeHint,
	LPFILE_ID_DESCRIPTOR  lpFileId,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwFlagsAndAttributes
	);

pCreateFile2  OrigCreateFile2 = NULL;
pCreateFileA  OrigCreateFileA = NULL;
pCreateFileW  OrigCreateFileW = NULL;
pOpenFileById OrigOpenFileById = NULL;


inline void GatherFileInfo(HANDLE fileHandle, gather_flag_t funcCalled)
{
    GatherInfo* tmpInfo = FileHandleToInfoObject(fileHandle, funcCalled);
    if (tmpInfo != NULL) {
	    gatherer->AddToBuff(FileHandleToInfoObject(fileHandle, funcCalled));
    }
}

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


typedef HMODULE(WINAPI *pLoadLibraryA)(
	LPCSTR lpLibFileName
	);
typedef HMODULE(WINAPI *pLoadLibraryW)(
	LPCWSTR lpLibFileName
	);
typedef HMODULE(WINAPI *pLoadLibraryExA)(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD  dwFlags
	);
typedef HMODULE(WINAPI *pLoadLibraryExW)(
	LPCWSTR lpLibFileName,
	HANDLE  hFile,
	DWORD   dwFlags
	);

pLoadLibraryA OrigLoadLibraryA = NULL;
pLoadLibraryW OrigLoadLibraryW = NULL;
pLoadLibraryExA OrigLoadLibraryExA = NULL;
pLoadLibraryExW OrigLoadLibraryExW = NULL;


inline void GatherLibraryInfo(HMODULE libHmodule, gather_flag_t funcCalled)
{
    GatherInfo* tmpInfo = LibraryHmoduleToInfoObject(libHmodule, funcCalled);
    if (tmpInfo != NULL) {
        gatherer->AddToBuff(LibraryHmoduleToInfoObject(libHmodule, funcCalled));
    }
}

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


typedef void(WINAPI *pExitProcess) (
	UINT ExitProcess
	);

pExitProcess OrigExitProcess = NULL;

void WINAPI NewExitProcess(UINT uExitCode) {
	WaitForSingleObject(freeLibSemaphore, 0);

	ReleaseSemaphore(waiterSemaphore, 1, NULL);

	WaitForSingleObject(waiterThread, INFINITE);

	OrigExitProcess(uExitCode);
}

bool GetOrigAddresses()
{
	if ((OrigCreateFile2 = (pCreateFile2)DetourFindFunction("kernel32.dll", "CreateFile2")) == NULL) {
		return false;
	}
	if ((OrigCreateFileA = (pCreateFileA)DetourFindFunction("kernel32.dll", "CreateFileA")) == NULL) {
		return false;
	}
	if ((OrigCreateFileW = (pCreateFileW)DetourFindFunction("kernel32.dll", "CreateFileW")) == NULL) {
		return false;
	}
	if ((OrigOpenFileById = (pOpenFileById)DetourFindFunction("kernel32.dll", "OpenFileById")) == NULL) {
		return false;
	}

	if ((OrigLoadLibraryA = (pLoadLibraryA)DetourFindFunction("kernel32.dll", "LoadLibraryA")) == NULL) {
		return false;
	}
	if ((OrigLoadLibraryW = (pLoadLibraryW)DetourFindFunction("kernel32.dll", "LoadLibraryW")) == NULL) {
		return false;
	}
	if ((OrigLoadLibraryExA = (pLoadLibraryExA)DetourFindFunction("kernel32.dll", "LoadLibraryExA")) == NULL) {
		return false;
	}
	if ((OrigLoadLibraryExW = (pLoadLibraryExW)DetourFindFunction("kernel32.dll", "LoadLibraryExW")) == NULL) {
		return false;
	}
	
	if ((OrigExitProcess = (pExitProcess)DetourFindFunction("kernel32.dll", "ExitProcess")) == NULL) {
		return false;
	}

	return true;
}

bool DetourExitProcess() {
    DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)OrigExitProcess, NewExitProcess);
	return DetourTransactionCommit() == NO_ERROR;
}

void UndetourExitProcess() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)OrigExitProcess, NewExitProcess);
    DetourTransactionCommit();
}

bool DetourFuncs() {
    if (DetourTransactionBegin() != NO_ERROR) {
		return false;
	}
	if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourAttach(&(PVOID&)OrigCreateFile2, NewCreateFile2) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigCreateFileA, NewCreateFileA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigCreateFileW, NewCreateFileW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigOpenFileById, NewOpenFileById) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourAttach(&(PVOID&)OrigLoadLibraryA, NewLoadLibraryA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigLoadLibraryW, NewLoadLibraryW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigLoadLibraryExA, NewLoadLibraryExA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigLoadLibraryExW, NewLoadLibraryExW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourTransactionCommit() != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	return true;
}

bool UndetourFuncs() {
    if (DetourTransactionBegin() != NO_ERROR) {
		return false;
	}
	if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourDetach(&(PVOID&)OrigCreateFile2, NewCreateFile2) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigCreateFileA, NewCreateFileA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigCreateFileW, NewCreateFileW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigOpenFileById, NewOpenFileById) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourDetach(&(PVOID&)OrigLoadLibraryA, NewLoadLibraryA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigLoadLibraryW, NewLoadLibraryW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigLoadLibraryExA, NewLoadLibraryExA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigLoadLibraryExW, NewLoadLibraryExW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourTransactionCommit() != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	return true;
}
