#pragma once
#include "stdafx.h"

#include "flags.h"


extern HANDLE senderThread;

void GatherFileInfo(HANDLE fileName, gather_flag_t funcCalled);

void GatherLibraryInfo(HMODULE libHmodule, gather_flag_t funcCalled);
