#pragma once

#include "stdafx.h"


typedef HANDLE(*pCreateFile2)(
	LPCWSTR                           lpFileName,
	DWORD                             dwDesiredAccess,
	DWORD                             dwShareMode,
	DWORD                             dwCreationDisposition,
	LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams
	);
typedef HANDLE(*pCreateFileA)(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
	);
typedef HANDLE(*pCreateFileW)(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
	);
typedef HFILE(*pOpenFile)(
	LPCSTR     lpFileName,
	LPOFSTRUCT lpReOpenBuff,
	UINT       uStyle
	);
typedef HANDLE(*pOpenFileById)(
	HANDLE                hVolumeHint,
	LPFILE_ID_DESCRIPTOR  lpFileId,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwFlagsAndAttributes
	);

extern pCreateFile2  OrigCreateFile2;
extern pCreateFileA  OrigCreateFileA;
extern pCreateFileW  OrigCreateFileW;
extern pOpenFile	 OrigOpenFile;
extern pOpenFileById OrigOpenFileById;

HANDLE NewCreateFile2(
	LPCWSTR                           lpFileName,
	DWORD                             dwDesiredAccess,
	DWORD                             dwShareMode,
	DWORD                             dwCreationDisposition,
	LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams
);

HANDLE NewCreateFileA(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
);

HANDLE NewCreateFileW(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
);

HFILE NewOpenFile(
	LPCSTR     lpFileName,
	LPOFSTRUCT lpReOpenBuff,
	UINT       uStyle
);

HANDLE NewOpenFileById(
	HANDLE                hVolumeHint,
	LPFILE_ID_DESCRIPTOR  lpFileId,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwFlagsAndAttributes
);



typedef HMODULE(*pLoadLibraryA)(
	LPCSTR lpLibFileName
	);
typedef HMODULE(*pLoadLibraryW)(
	LPCWSTR lpLibFileName
	);
typedef HMODULE(*pLoadLibraryExA)(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD  dwFlags
	);
typedef HMODULE(*pLoadLibraryExW)(
	LPCWSTR lpLibFileName,
	HANDLE  hFile,
	DWORD   dwFlags
	);

extern pLoadLibraryA OrigLoadLibraryA;
extern pLoadLibraryW OrigLoadLibraryW;
extern pLoadLibraryExA OrigLoadLibraryExA;
extern pLoadLibraryExW OrigLoadLibraryExW;

HMODULE NewLoadLibraryA(
	LPCSTR lpLibFileName
);

HMODULE NewLoadLibraryW(
	LPCWSTR lpLibFileName
);

HMODULE NewLoadLibraryExA(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD  dwFlags
);

HMODULE NewLoadLibraryExW(
	LPCWSTR lpLibFileName,
	HANDLE  hFile,
	DWORD   dwFlags
);
