#pragma once

#include "flags.h"

typedef UINT32 buff_size_t;


const int MAX_NAME_LENGTH = 38000;


class GatherInfo {
public:
	GatherInfo(gather_flag_t type, gather_flag_t funcCalled);
	GatherInfo(gather_flag_t type, gather_flag_t funcCalled, LPWSTR name, UINT32 nameLength);

	~GatherInfo();

	const buff_size_t GetSize() const;
	PBYTE ToMessageFormat() const;

private:
	gather_flag_t type;
	gather_flag_t funcCalled;
	UINT32 nameLength;
	LPWSTR name;
};

GatherInfo * FileHandleToInfoObject(HANDLE fileHandle, gather_flag_t funcCalled);
GatherInfo * LibraryHmoduleToInfoObject(HMODULE libHmodule, gather_flag_t funcCalled);
