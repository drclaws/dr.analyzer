#include "stdafx.h"
#include "GatherInfo.h"

#include <cstring>
#include <stdio.h>

#include "flags.h"


GatherInfo::GatherInfo() {
	this->type = GatherType::GatherNone;
	this->funcCalled = GatherFuncType::GatherUnknownFunc;
	this->nameLength = 0;
	this->name = NULL;
}

GatherInfo::~GatherInfo() {
	if (this->name) {
		delete[] this->name;
	}
}

GatherInfo::GatherInfo(gather_flag_t type, gather_flag_t funcCalled) {
	this->type = type;
	this->funcCalled = funcCalled;
	this->nameLength = 0;
	this->name = NULL;
}

GatherInfo::GatherInfo(gather_flag_t type, gather_flag_t funcCalled, INT32 emergencyCode) {
	this->type = type;
	this->funcCalled = funcCalled;
	this->nameLength = emergencyCode;
	this->name = NULL;
}

GatherInfo::GatherInfo(gather_flag_t type, gather_flag_t funcCalled, LPWSTR name, INT32 nameLength) {
	this->type = type;
	this->funcCalled = funcCalled;
	this->nameLength = nameLength;
	this->name = name;
}

buff_size_t GatherInfo::GetSize() {
	if (this->nameLength > 0) {
		return sizeof(this->type) + sizeof(this->funcCalled) + sizeof(this->nameLength) + this->nameLength * sizeof(WCHAR);
	}
	else if (this->nameLength < 0)
	{
		return sizeof(this->type) + sizeof(this->funcCalled) + sizeof(this->nameLength);
	}
	else {
		return sizeof(this->type) + sizeof(this->funcCalled);
	}
}

INT8* GatherInfo::ToMessageFormat()
{
	buff_size_t sizeBuff = this->GetSize();
	INT8* buff = new INT8[sizeBuff];
	
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

void GatherInfo::Print()
{
	if (this->name) {
		std::wcout << this->type << " (" << this->funcCalled << "): " << this->nameLength << " " << this->name << std::endl;
	}
	else {
		std::wcout << this->type << " (" << this->funcCalled << "): " << this->nameLength << std::endl;
	}
}

GatherInfo * FileHandleToInfoObject(HANDLE fileHandle, gather_flag_t funcCalled)
{
	const DWORD size = 255;
	WCHAR filePath[size + 1];

	DWORD sizeGet;
	LPWSTR filePathRes;

	sizeGet = GetFinalPathNameByHandleW(fileHandle, filePath, size, FILE_NAME_NORMALIZED);

	if (sizeGet == 0) {
		return new GatherInfo(GatherType::GatherFile, funcCalled, GatherWarning::GatherCannotGetValue);
	}
	if (sizeGet > MAX_NAME_LENGTH) {
		return new GatherInfo(GatherType::GatherFile, funcCalled, GatherWarning::GatherNameTooBig);
	}

	if (sizeGet > size) {
		filePathRes = new WCHAR[sizeGet];
		sizeGet = GetFinalPathNameByHandleW(fileHandle, filePathRes, sizeGet - (DWORD)1, FILE_NAME_NORMALIZED);
	}
	else {
		filePathRes = new WCHAR[sizeGet + 1Ui64];
		std::memcpy(filePathRes, filePath, sizeof(WCHAR) * (sizeGet + 1Ui64));
	}

	return new GatherInfo(GatherType::GatherFile, funcCalled, filePathRes, sizeGet);
}

GatherInfo * LibraryHmoduleToInfoObject(HMODULE libHmodule, gather_flag_t funcCalled)
{
	const DWORD size = 512;
	WCHAR filePath[size];

	DWORD sizeGet;
	LPWSTR filePathRes;

	sizeGet = GetModuleFileNameW(libHmodule, filePath, size);

	if (sizeGet == 0) {
		return new GatherInfo(GatherType::GatherLibrary, funcCalled, GatherWarning::GatherCannotGetValue);
	}
	else if (sizeGet != size) {
		sizeGet++;
	}

	filePathRes = new WCHAR[sizeGet];
	std::memcpy(filePathRes, filePath, sizeof(WCHAR) * sizeGet);

	return new GatherInfo(GatherType::GatherLibrary, funcCalled, filePathRes, sizeGet - 1);
}
