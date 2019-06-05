#include "stdafx.h"
#include "hook_funcs.h"

#include "hook.h"

#include <detours.h>


pCreateFile2  OrigCreateFile2 = NULL;
pCreateFileA  OrigCreateFileA = NULL;
pCreateFileW  OrigCreateFileW = NULL;
pOpenFile	  OrigOpenFile = NULL;
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
		GatherFileInfo((void*)fileHandle);
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
		GatherFileInfo((void*)fileHandle);
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
		GatherFileInfo((void*)fileHandle);
	}

	return fileHandle;
}

HFILE WINAPI NewOpenFile(
	LPCSTR     lpFileName,
	LPOFSTRUCT lpReOpenBuff,
	UINT       uStyle
) {
	HFILE fileHandle = OrigOpenFile(
						   lpFileName,
						   lpReOpenBuff,
						   uStyle);

	if (fileHandle != NULL) {
		GatherFileInfo((void*)fileHandle);
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
		GatherFileInfo((void*)fileHandle);
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
	HMODULE libHandle = OrigLoadLibraryA(lpLibFileName);

	if (libHandle != NULL) {
		GatherLibraryInfo((void*)libHandle);
	}

	return libHandle;
}

HMODULE WINAPI NewLoadLibraryW(
	LPCWSTR lpLibFileName
) {
	HMODULE libHandle = OrigLoadLibraryW(lpLibFileName);

	if (libHandle != NULL) {
		GatherLibraryInfo((void*)libHandle);
	}

	return libHandle;
}

HMODULE WINAPI NewLoadLibraryExA(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD  dwFlags
) {
	HMODULE libHandle = OrigLoadLibraryExA(
							lpLibFileName,
							hFile,
							dwFlags);

	if (libHandle != NULL) {
		GatherLibraryInfo((void*)libHandle);
	}

	return libHandle;
}

HMODULE WINAPI NewLoadLibraryExW(
	LPCWSTR lpLibFileName,
	HANDLE  hFile,
	DWORD   dwFlags
) {
	HMODULE libHandle = OrigLoadLibraryExW(
							lpLibFileName,
							hFile,
							dwFlags);

	if (libHandle != NULL) {
		GatherLibraryInfo((void*)libHandle);
	}

	return libHandle;
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
	if ((OrigOpenFile = (pOpenFile)DetourFindFunction("kernel32.dll", "OpenFile")) == NULL) {
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

	return TRUE;
}
