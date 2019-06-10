#include "stdafx.h"
#include "hook.h"

#include <exception>

#include "Gatherer.h"
#include "GatherInfo.h"
#include "flags.h"

#include <iostream>


void GatherActivate() {
	gatherer = new Gatherer();
	gatherer->Activate();
}

void GatherDeactivate() {
	delete gatherer;
}

void GatherFileInfo(HANDLE fileHandle, gather_flag_t funcCalled)
{
	gatherer->AddToBuff(FileHandleToInfoObject(fileHandle, funcCalled));
}

void GatherLibraryInfo(HMODULE libHmodule, gather_flag_t funcCalled)
{
	gatherer->AddToBuff(LibraryHmoduleToInfoObject(libHmodule, funcCalled));
}
