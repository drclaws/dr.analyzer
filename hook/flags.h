#pragma once
#include "stdafx.h"

typedef UINT16 gather_flag_t;


enum GatherFuncType {
	GatherUnknownFunc =		(gather_flag_t)0x0000,

	GatherFileFunc =		(gather_flag_t)0x1000,
	GatherLibraryFunc =		(gather_flag_t)0x2000,

	GatherLoadFunc =		(gather_flag_t)0x4000,


	GatherCreateFile2 =		GatherFileFunc | (gather_flag_t)0x0001,
	GatherCreateFileA =		GatherFileFunc | (gather_flag_t)0x0002,
	GatherCreateFileW =		GatherFileFunc | (gather_flag_t)0x0004,
	GatherOpenFileById =	GatherFileFunc | (gather_flag_t)0x0008,

	GatherLoadLibraryA =	GatherLibraryFunc | (gather_flag_t)0x0001,
	GatherLoadLibraryW =	GatherLibraryFunc | (gather_flag_t)0x0002,
	GatherLoadLibraryExA =	GatherLibraryFunc | (gather_flag_t)0x0004,
	GatherLoadLibraryExW =	GatherLibraryFunc | (gather_flag_t)0x0008,

	GatherConnection =		GatherLoadFunc | (gather_flag_t)0x0001,
	GatherFilesOnLoad =		GatherLoadFunc | (gather_flag_t)0x0002,
	GatherWaiter =			GatherLoadFunc | (gather_flag_t)0x0004,
	GatherBufferSender =	GatherLoadFunc | (gather_flag_t)0x0008
};

enum GatherType {
	GatherNone =		(gather_flag_t)0x0000,

	GatherStatus =		(gather_flag_t)0x1000,
	GatherResource =	(gather_flag_t)0x2000,


	GatherActivated =	GatherStatus | (gather_flag_t)0x0001,
	GatherDeactivated = GatherStatus | (gather_flag_t)0x0002,
	GatherStillUp =		GatherStatus | (gather_flag_t)0x0004,
	GatherWaiterError = GatherStatus | (gather_flag_t)0x0008,
	GatherDetourError = GatherStatus | (gather_flag_t)0x0010,

	GatherFile =		GatherResource | (gather_flag_t)0x0001,
	GatherLibrary =		GatherResource | (gather_flag_t)0x0002
};
