#include "stdafx.h"
#include "BuffObject.h"


BuffObject::BuffObject()
{
	this->buffer = new GatherInfo*[this->maxLength];
}


BuffObject::~BuffObject()
{
	for (int i = 0; i < this->size; i++) {
		delete this->buffer[i];
	}

	delete this->buffer;
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
