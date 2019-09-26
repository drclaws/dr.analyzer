#include "stdafx.h"
#include "BuffObject.hpp"

#include <cstring>


BuffObject::BuffObject() {
    
}


BuffObject::~BuffObject()
{
	for (int i = 0; i < this->length; i++) {
		delete this->buffer[i];
	}
}

const bool BuffObject::AddInfo(GatherInfo * info)
{
	int size;

	if (!(this->length < BUFF_MAX_LENGTH) || !((size = info->GetSize()) < BUFF_MAX_SIZE)) {
		return false;
	}
	
	this->buffer[this->length] = info;
	
	this->length++;
	this->size += size;

	return true;
}

const bool BuffObject::IsEmpty() const
{
	return this->length == 0;
}

PBYTE BuffObject::ToMessage() const
{
    buff_size_t messageSize = this->MessageSize();
	PBYTE message = new BYTE[messageSize];
	
	std::memcpy(message, &this->size, sizeof(this->size));
	
	int limit = this->length / 2;
	PBYTE currLeftPos = (PBYTE)((ULONG_PTR)message + (ULONG_PTR)sizeof(this->size)),
	    currRightPos = (PBYTE)((ULONG_PTR)message + (ULONG_PTR)messageSize);
	
	for(int i = 0, j = this->length - 1; i < limit; i++, j--) {
	    buff_size_t currLeftBuffSize = this->buffer[i]->GetSize(),
	        currRightBuffSize = this->buffer[j]->GetSize();
	    PBYTE currLeftBuff = this->buffer[i]->ToMessageFormat(),
	        currRightBuff = this->buffer[j]->ToMessageFormat();
	    
	    std::memcpy(currLeftPos, currLeftBuff, currLeftBuffSize);
	    
	    currLeftPos = (PBYTE)((ULONG_PTR)currLeftPos + (ULONG_PTR)currLeftBuffSize);
	    currRightPos = (PBYTE)((ULONG_PTR)currRightPos - (ULONG_PTR)currRightBuffSize);
	    
	    std::memcpy(currRightPos, currRightBuff, currRightBuffSize);
	    
	    (delete [] currLeftBuff, delete [] currRightBuff);
	}
	
    if (this->length % 2 != 0) {
        buff_size_t currLeftBuffSize = this->buffer[limit]->GetSize();
        PBYTE currLeftBuff = this->buffer[limit]->ToMessageFormat();
        
        std::memcpy(currLeftPos, currLeftBuff, currLeftBuffSize);
        
        delete [] currLeftBuff;
    }
    
	return message;
}

const buff_size_t BuffObject::MessageSize() const
{
	return this->size + sizeof(buff_size_t);
}
