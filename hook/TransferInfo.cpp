#include "stdafx.h"
#include "TransferInfo.h"


TransferInfo::TransferInfo() {
	this->Type = TransNone;
	this->nameLength = 0;
	this->name = NULL;
}

TransferInfo::~TransferInfo() {
	if (this->name) {
		delete[] this->name;
	}
}

TransferInfo::TransferInfo(INT8 type) {
	this->Type = type;
	this->nameLength = 0;
	this->name = NULL;
}

TransferInfo::TransferInfo(INT8 type, LPCWSTR name, INT32 nameLength) {
	this->Type = type;
	this->nameLength = nameLength;
	this->name = name;
}

INT32 TransferInfo::GetSize() {
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
