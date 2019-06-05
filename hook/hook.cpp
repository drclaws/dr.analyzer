#include "stdafx.h"
#include "hook.h"

#include <exception>

#include "Gatherer.h"
#include "TransferInfo.h"


Gatherer* gatherer;

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

void GatherFileInfo(void *fileHandle)
{
	gatherer->AddFileToBuff((HANDLE)fileHandle);
}

void GatherLibraryInfo(void *libHandle)
{
	gatherer->AddLibToBuff((HANDLE)libHandle);
}

void WarningFileNameToBig()
{
	gatherer->WarningNameToBig(TransFileOpen);
}

void WarningLibraryNameToBig()
{
	gatherer->WarningNameToBig(TransLibraryOpen);
}
