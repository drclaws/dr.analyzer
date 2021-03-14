#include "stdafx.h"
#include "GatherInfo.hpp"

#include <cstdlib>
#include <cstring>
#include <cstdint>

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

GatherInfo::GatherInfo(gather_flag_t type, gather_flag_t funcCalled, LPWSTR name, UINT32 nameLength) {
	this->type = type | GatherType::GatherHasValue;
	this->funcCalled = funcCalled;
	this->nameLength = nameLength;
	this->name = name;
}

const buff_size_t GatherInfo::GetSize() const {
	return this->nameLength > 0
	    ? sizeof(this->type) + sizeof(this->funcCalled) + sizeof(this->nameLength) + this->nameLength * sizeof(WCHAR)
	    : sizeof(this->type) + sizeof(this->funcCalled);
}

PBYTE GatherInfo::ToMessageFormat() const
{
	buff_size_t sizeBuff = this->GetSize();
    uint8_t* buff = new uint8_t[sizeBuff];
	
	std::memcpy((void*)buff, &this->type, sizeof(this->type));
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

    LPWSTR filePathRes;
    if ((filePathRes = static_cast<LPWSTR>(std::malloc(sizeof(WCHAR) * sizeSet))) == NULL)
        return nullptr; // TODO Send error of memory allocation
    
    sizeGet = GetFinalPathNameByHandleW(fileHandle, filePathRes, sizeSet, FILE_NAME_NORMALIZED);
    
    if (sizeGet == 0) {
        std::free(filePathRes);
        return nullptr; // TODO Send error of getting file name
    }
    
    if (sizeGet >= sizeSet) {
    	std::free(filePathRes);
        if (sizeGet > MAX_NAME_LENGTH) {
            return new GatherInfo(GatherType::GatherFilePathToBig, funcCalled);
        }

        if ((filePathRes = static_cast<LPWSTR>(std::malloc(sizeof(WCHAR) * sizeSet))) == NULL)
            return nullptr;

    	sizeGet = GetFinalPathNameByHandleW(fileHandle, filePathRes, sizeGet, FILE_NAME_NORMALIZED);
    }

    auto finalPathRes = static_cast<LPWSTR>(std::realloc(filePathRes, sizeof(WCHAR) * sizeGet));

    if (finalPathRes == nullptr)
    {
        std::free(filePathRes);
        return nullptr; // TODO Send error of memory allocation
    }
    else
        return new GatherInfo(GatherType::GatherFile, funcCalled, finalPathRes, sizeGet);
}

GatherInfo * LibraryHmoduleToInfoObject(HMODULE libHmodule, gather_flag_t funcCalled)
{	
	DWORD sizeGet, sizeSet = baseArraySize;
    LPWSTR filePathRes;
    
    if ((filePathRes = static_cast<LPWSTR>(std::malloc(sizeSet * sizeof(WCHAR)))) == NULL)
        return nullptr; // TODO Send error of memory allocation

    while((sizeGet = GetModuleFileNameW(libHmodule, filePathRes, sizeSet)) == sizeSet) {
        std::free(filePathRes);
        if(sizeSet == MAX_NAME_LENGTH)
            return new GatherInfo(GatherType::GatherModulePathToBig, funcCalled);
        DWORD newSize = sizeSet * 2 + 1;
        sizeSet = newSize > MAX_NAME_LENGTH
            ? MAX_NAME_LENGTH
            : newSize;
        filePathRes = static_cast<LPWSTR>(std::malloc(sizeSet * sizeof(WCHAR)));
        if (filePathRes == nullptr)
            return nullptr; // TODO Send error of memory allocation
    }
    
    if(sizeGet == 0) {
        return nullptr; // TODO Send error of getting file name
    }
    
    auto finalPathRes = static_cast<LPWSTR>(std::realloc(filePathRes, sizeGet * sizeof(WCHAR)));

    if (finalPathRes == nullptr)
    {
        std::free(filePathRes);
        return nullptr; // TODO Send error of memory allocation
    }
    else
        return new GatherInfo(GatherType::GatherLibrary, funcCalled, finalPathRes, sizeGet);
}
