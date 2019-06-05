#include "stdafx.h"
#include "hook_funcs.h"

#include "hook.h"


pCreateFile2  OrigCreateFile2 = NULL;
pCreateFileA  OrigCreateFileA = NULL;
pCreateFileW  OrigCreateFileW = NULL;
pOpenFile	  OrigOpenFile = NULL;
pOpenFileById OrigOpenFileById = NULL;

HANDLE NewCreateFile2(
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

HANDLE NewCreateFileA(
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

HANDLE NewCreateFileW(
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

HFILE NewOpenFile(
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

HANDLE NewOpenFileById(
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

HMODULE NewLoadLibraryA(
	LPCSTR lpLibFileName
) {
	HMODULE libHandle = OrigLoadLibraryA(lpLibFileName);

	if (libHandle != NULL) {
		GatherLibraryInfo((void*)libHandle);
	}

	return libHandle;
}

HMODULE NewLoadLibraryW(
	LPCWSTR lpLibFileName
) {
	HMODULE libHandle = OrigLoadLibraryW(lpLibFileName);

	if (libHandle != NULL) {
		GatherLibraryInfo((void*)libHandle);
	}

	return libHandle;
}

HMODULE NewLoadLibraryExA(
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

HMODULE NewLoadLibraryExW(
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
