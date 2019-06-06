#pragma once
#include "stdafx.h"

#include "flags.h"

// Prepares connection and info gathering
int StartGathering();

// Stops info gathering and closes connection
int StopGathering();

void GatherFileInfo(HANDLE fileName, gather_flag_t funcCalled);

void GatherLibraryInfo(HMODULE libHmodule, gather_flag_t funcCalled);
