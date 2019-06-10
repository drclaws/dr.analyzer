#pragma once
#include "stdafx.h"

#include "flags.h"

void GatherActivate();

void GatherDeactivate();

void GatherFileInfo(HANDLE fileName, gather_flag_t funcCalled);

void GatherLibraryInfo(HMODULE libHmodule, gather_flag_t funcCalled);
