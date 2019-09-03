#include "stdafx.h"
#include "BuffObject.h"

#include <cstring>


BuffObject::BuffObject() {
	this->buffer = new GatherInfo * [BUFF_MAX_LENGTH];
}


BuffObject::~BuffObject()
{
	for (int i = 0; i < this->length; i++) {
		delete this->buffer[i];
	}
	delete[] this->buffer;
}

bool BuffObject::AddInfo(GatherInfo * info)
{
	int size;

	if (!(this->remainsLength > 0) || (size = info->GetSize()) > this->remainsSize) {
		return false;
	}
	
	this->buffer[this->length] = info;
	this->length++;
	this->remainsLength--;

	this->size += size;
	this->remainsSize -= size;

	return true;
}

bool BuffObject::IsEmpty()
{
	return this->length == 0;
}

PBYTE BuffObject::ToMessage()
{
	PBYTE message = new BYTE[this->MessageSize()];
	
	std::memcpy(message, &this->size, sizeof(this->size));
	
	PBYTE currPos = (PBYTE)((ULONG_PTR)message + sizeof(this->size));
	buff_size_t currBuffSize;
	PBYTE currBuff;

	for (int i = 0; i < this->length; i++) {
		currBuffSize = this->buffer[i]->GetSize();
		currBuff = this->buffer[i]->ToMessageFormat();
		std::memcpy(currPos, currBuff, currBuffSize);
		
		currPos = (PBYTE)((ULONG_PTR)currPos + (ULONG_PTR)currBuffSize);
		delete [] currBuff;
	}

	return message;
}

buff_size_t BuffObject::MessageSize()
{
	return this->size + sizeof(buff_size_t);
}
