#include "stdafx.h"
#include "TransferInfo.h"


TransferInfo::TransferInfo() {
	this->Type = TransNone;
	this->nameLength = 0;
	this->name = NULL;
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
