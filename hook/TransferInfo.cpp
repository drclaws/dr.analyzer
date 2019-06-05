#include "stdafx.h"
#include "TransferInfo.h"


GatherInfo::GatherInfo() {
	this->Type = GatherType::GatherNone;
	this->nameLength = 0;
	this->name = NULL;
}

GatherInfo::~GatherInfo() {
	if (this->name) {
		delete[] this->name;
	}
}

GatherInfo::GatherInfo(INT8 type) {
	this->Type = type;
	this->nameLength = 0;
	this->name = NULL;
}

GatherInfo::GatherInfo(INT8 type, LPCWSTR name, INT32 nameLength) {
	this->Type = type;
	this->nameLength = nameLength;
	this->name = name;
}

INT32 GatherInfo::GetSize() {
	const INT32 typesSize = sizeof(this->Type) + sizeof(this->nameLength);
	if (this->nameLength > 0) {
		return typesSize + this->nameLength * sizeof(WCHAR);
		
	}
	else if (this->nameLength == 0) {
		return sizeof(this->Type);
	}
	else {
		return typesSize;
	}
}
