#include "stdafx.h"

#include "hook_funcs.h"


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
	HANDLE file_handle = OrigCreateFile2(
							 lpFileName, 
							 dwDesiredAccess, 
							 dwShareMode,
							 dwCreationDisposition,
							 pCreateExParams);

	return file_handle;
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
	HANDLE file_handle = OrigCreateFileA(
							 lpFileName,
							 dwDesiredAccess,
							 dwShareMode, 
							 lpSecurityAttributes, 
							 dwCreationDisposition, 
							 dwFlagsAndAttributes,
							 hTemplateFile);

	return file_handle;
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
	HANDLE file_handle = OrigCreateFileW(
							 lpFileName,
							 dwDesiredAccess,
							 dwShareMode,
							 lpSecurityAttributes,
							 dwCreationDisposition,
							 dwFlagsAndAttributes,
							 hTemplateFile);

	return file_handle;
}

HFILE NewOpenFile(
	LPCSTR     lpFileName,
	LPOFSTRUCT lpReOpenBuff,
	UINT       uStyle
) {
	HFILE file_handle = OrigOpenFile(
							lpFileName,
							lpReOpenBuff,
							uStyle);

	return file_handle;
}

HANDLE NewOpenFileById(
	HANDLE                hVolumeHint,
	LPFILE_ID_DESCRIPTOR  lpFileId,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwFlagsAndAttributes
) {
	HANDLE file_handle = OrigOpenFileById(
							 hVolumeHint,
							 lpFileId,
							 dwDesiredAccess,
							 dwShareMode,
							 lpSecurityAttributes,
							 dwFlagsAndAttributes);

	return file_handle;
}



pLoadLibraryA OrigLoadLibraryA = NULL;
pLoadLibraryW OrigLoadLibraryW = NULL;
pLoadLibraryExA OrigLoadLibraryExA = NULL;
pLoadLibraryExW OrigLoadLibraryExW = NULL;

HMODULE NewLoadLibraryA(
	LPCSTR lpLibFileName
) {
	HMODULE lib_handle = OrigLoadLibraryA(lpLibFileName);

	return lib_handle;
}

HMODULE NewLoadLibraryW(
	LPCWSTR lpLibFileName
) {
	HMODULE lib_handle = OrigLoadLibraryW(lpLibFileName);

	return lib_handle;
}

HMODULE NewLoadLibraryExA(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD  dwFlags
) {
	HMODULE lib_handle = OrigLoadLibraryExA(
							 lpLibFileName,
							 hFile,
							 dwFlags);

	return lib_handle;
}

HMODULE NewLoadLibraryExW(
	LPCWSTR lpLibFileName,
	HANDLE  hFile,
	DWORD   dwFlags
) {
	HMODULE lib_handle = OrigLoadLibraryExW(
							 lpLibFileName,
							 hFile,
							 dwFlags);

	return lib_handle;
}



pCloseHandle OrigCloseHandle = NULL;

BOOL NewCloseHandle(
	HANDLE hObject
) {
	BOOL is_closed = OrigCloseHandle(hObject);

	return is_closed;
}
