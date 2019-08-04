#pragma once
#include "stdafx.h"

#include "flags.h"


extern HMODULE libHModule;

extern HANDLE waiterThread;
extern HANDLE waiterSemaphore;

extern HANDLE freeLibSemaphore;

void GatherFileInfo(HANDLE fileName, gather_flag_t funcCalled);

void GatherLibraryInfo(HMODULE libHmodule, gather_flag_t funcCalled);
