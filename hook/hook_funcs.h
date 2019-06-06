#pragma once

#include "stdafx.h"


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

extern pCreateFile2  OrigCreateFile2;
extern pCreateFileA  OrigCreateFileA;
extern pCreateFileW  OrigCreateFileW;
extern pOpenFileById OrigOpenFileById;

HANDLE WINAPI NewCreateFile2(
	LPCWSTR                           lpFileName,
	DWORD                             dwDesiredAccess,
	DWORD                             dwShareMode,
	DWORD                             dwCreationDisposition,
	LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams
);

HANDLE WINAPI NewCreateFileA(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
);

HANDLE WINAPI NewCreateFileW(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
);

HANDLE WINAPI NewOpenFileById(
	HANDLE                hVolumeHint,
	LPFILE_ID_DESCRIPTOR  lpFileId,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwFlagsAndAttributes
);



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

extern pLoadLibraryA OrigLoadLibraryA;
extern pLoadLibraryW OrigLoadLibraryW;
extern pLoadLibraryExA OrigLoadLibraryExA;
extern pLoadLibraryExW OrigLoadLibraryExW;

HMODULE WINAPI NewLoadLibraryA(
	LPCSTR lpLibFileName
);

HMODULE WINAPI NewLoadLibraryW(
	LPCWSTR lpLibFileName
);

HMODULE WINAPI NewLoadLibraryExA(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD  dwFlags
);

HMODULE WINAPI NewLoadLibraryExW(
	LPCWSTR lpLibFileName,
	HANDLE  hFile,
	DWORD   dwFlags
);


BOOL GetOrigAddresses();
