#pragma once

#include "flags.h"


const int MAX_NAME_LENGTH = 38000;


enum GatherWarning {
	GatherNameTooBig = (INT32)-1,
	GatherCannotGetValue = (INT32)-2
};


class GatherInfo {
public:
	GatherInfo();
	~GatherInfo();

	GatherInfo(gather_flag_t type, gather_flag_t funcCalled, INT32 emergencyCode=0);
	GatherInfo(gather_flag_t type, gather_flag_t funcCalled, LPWSTR name, INT32 nameLength);
	
	INT32 GetSize();
	void* ToMessageFormat();

private:
	gather_flag_t type;
	gather_flag_t funcCalled;
	INT32 nameLength;
	LPWSTR name;
};

GatherInfo * FileHandleToInfoObject(HANDLE fileHandle, gather_flag_t funcCalled);
GatherInfo * LibraryHmoduleToInfoObject(HMODULE libHmodule, gather_flag_t funcCalled);
