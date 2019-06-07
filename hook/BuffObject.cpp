#include "stdafx.h"
#include "BuffObject.h"

#include <cstring>


BuffObject::BuffObject() { }


BuffObject::~BuffObject()
{
	for (int i = 0; i < this->length; i++) {
		delete this->buffer[i];
	}
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

INT8* BuffObject::ToMessage()
{
	INT8* message = new INT8[this->size + sizeof(buff_size_t)];
	
	std::memcpy(message, &this->size, sizeof(this->size));
	
	buff_size_t currPos = sizeof(this->size);
	buff_size_t currBuffSize;
	INT8 *currBuff;

	for (int i = 0; i < this->length; i++) {
		currBuffSize = this->buffer[i]->GetSize();
		currBuff = this->buffer[i]->ToMessageFormat();
		std::memcpy(message + currPos, currBuff, currBuffSize);
		
		currPos += currBuffSize;
		delete [] currBuff;
	}

	return message;
}
