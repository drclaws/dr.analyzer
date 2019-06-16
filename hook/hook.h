#pragma once
#include "stdafx.h"

#include "flags.h"

extern HMODULE libHModule;
extern HANDLE gatherThread;

extern HANDLE waiterThread;
extern HANDLE waiterSemaphore;

void GatherFileInfo(HANDLE fileName, gather_flag_t funcCalled);

void GatherLibraryInfo(HMODULE libHmodule, gather_flag_t funcCalled);
