#include "stdafx.h"
#include "GatherInfo.h"

#include <cstdlib>
#include <cstring>

#include "flags.h"


const DWORD baseArraySize = 255;


GatherInfo::~GatherInfo() {
	if (this->name) {
		std::free(this->name);
	}
}

GatherInfo::GatherInfo(gather_flag_t type, gather_flag_t funcCalled) {
	this->type = type;
	this->funcCalled = funcCalled;
	this->nameLength = 0;
	this->name = NULL;
}

/*GatherInfo::GatherInfo(gather_flag_t type, gather_flag_t funcCalled, INT32 emergencyCode) {
	this->type = type;
	this->funcCalled = funcCalled;
	this->nameLength = emergencyCode;
	this->name = NULL;
}*/

GatherInfo::GatherInfo(gather_flag_t type, gather_flag_t funcCalled, LPWSTR name, UINT32 nameLength) {
	this->type = type | GatherType::GatherHasValue;
	this->funcCalled = funcCalled;
	this->nameLength = nameLength;
	this->name = name;
}

buff_size_t GatherInfo::GetSize() {
	/*if (this->nameLength > 0) {
		return sizeof(this->type) + sizeof(this->funcCalled) + sizeof(this->nameLength) + this->nameLength * sizeof(WCHAR);
	}
	else if (this->nameLength < 0)
	{
		return sizeof(this->type) + sizeof(this->funcCalled) + sizeof(this->nameLength);
	}
	else {
		return sizeof(this->type) + sizeof(this->funcCalled);
	}*/
	return this->nameLength > 0
	    ? sizeof(this->type) + sizeof(this->funcCalled) + sizeof(this->nameLength) + this->nameLength * sizeof(WCHAR)
	    : sizeof(this->type) + sizeof(this->funcCalled);
}

PBYTE GatherInfo::ToMessageFormat()
{
	buff_size_t sizeBuff = this->GetSize();
	PBYTE buff = new BYTE[sizeBuff];
	
	std::memcpy(buff, &this->type, sizeof(this->type));
	std::memcpy(buff + sizeof(this->type), &this->funcCalled, sizeof(this->funcCalled));
	if (this->nameLength != 0) {
		std::memcpy(buff + (sizeof(this->type) + sizeof(this->funcCalled)), &this->nameLength, sizeof(this->nameLength));
		if (this->nameLength > 0) {
			std::memcpy(buff + (sizeof(this->type) + sizeof(this->funcCalled) + sizeof(this->nameLength)), this->name, this->nameLength * sizeof(WCHAR));
		}
	}

	return buff;
}

GatherInfo * FileHandleToInfoObject(HANDLE fileHandle, gather_flag_t funcCalled)
{
	DWORD sizeGet, sizeSet = baseArraySize;
    LPWSTR filePathRes = (LPWSTR)std::malloc(sizeof(WCHAR) * sizeSet);
    
    sizeGet = GetFinalPathNameByHandleW(fileHandle, filePathRes, sizeSet - 1, FILE_NAME_NORMALIZED);
    
    if (sizeGet == 0) {
        std::free(filePathRes);
    	return NULL;
    }
    
    if (sizeGet > sizeSet) {
    	std::free(filePathRes);
        if (sizeGet > MAX_NAME_LENGTH) {
            return new GatherInfo(GatherType::GatherFilePathToBig, funcCalled);
        }
    	filePathRes = (LPWSTR)std::malloc(sizeof(WCHAR) * sizeGet);
    	sizeGet = GetFinalPathNameByHandleW(fileHandle, filePathRes, sizeGet - (DWORD)1, FILE_NAME_NORMALIZED);
    }
    else {
    	filePathRes = (LPWSTR)std::realloc(filePathRes, sizeof(WCHAR) * (sizeGet + 1));
    }
    
    return new GatherInfo(GatherType::GatherFile, funcCalled, filePathRes, sizeGet);
}

GatherInfo * LibraryHmoduleToInfoObject(HMODULE libHmodule, gather_flag_t funcCalled)
{	
	DWORD sizeGet, sizeSet = baseArraySize;
    LPWSTR filePathRes = (LPWSTR)std::malloc(sizeSet * sizeof(WCHAR));
        
    while((sizeGet = GetModuleFileNameW(libHmodule, filePathRes, sizeSet)) == sizeSet) {
        std::free(filePathRes);
        if(sizeSet == MAX_NAME_LENGTH) {
            return new GatherInfo(GatherType::GatherModulePathToBig, funcCalled);
        }
        sizeSet = sizeSet * 2 + 1;
        if(sizeSet > MAX_NAME_LENGTH) {
            sizeSet = MAX_NAME_LENGTH;
        }
        filePathRes = (LPWSTR)std::malloc(sizeSet * sizeof(WCHAR));
    }
    
    if(sizeGet == 0) {
        return NULL;
    }
    
    if(sizeSet != sizeGet + 1) {
        filePathRes = (LPWSTR)std::realloc(filePathRes, (sizeGet + 1) * sizeof(WCHAR));
    }
    
    return new GatherInfo(GatherType::GatherLibrary, funcCalled, filePathRes, sizeGet);
}
