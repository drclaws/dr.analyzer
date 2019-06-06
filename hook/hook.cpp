#include "stdafx.h"
#include "hook.h"

#include <exception>

#include "Gatherer.h"
#include "GatherInfo.h"
#include "flags.h"


int StartGathering()
{
	try {
		gatherer = new Gatherer();
	}
	catch(std::exception) {
		return FALSE;
	}
	return TRUE;
}

int StopGathering()
{
	try {
		delete gatherer;
	}
	catch (std::exception) {
		return FALSE;
	}
	return TRUE;
}

void GatherFileInfo(HANDLE fileHandle, gather_flag_t funcCalled)
{
	gatherer->AddToBuff(FileHandleToInfoObject(fileHandle, funcCalled));
}

void GatherLibraryInfo(HMODULE libHmodule, gather_flag_t funcCalled)
{
	gatherer->AddToBuff(LibraryHmoduleToInfoObject(libHmodule, funcCalled));
}
