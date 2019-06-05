#pragma once

#include "TransferInfo.h"

#define BUFF_MAX_LENGTH 500
#define BUFF_MAX_SIZE 76000


class BuffObject
{
public:
	const int maxLength = BUFF_MAX_LENGTH;
	const int maxSize = BUFF_MAX_SIZE;

	BuffObject();
	~BuffObject();

	bool AddInfo(GatherInfo* info);
	void* ToMessage();

private:
	int length = 0;
	int size = 0;

	int remainsLength = maxLength;
	int remainsSize = maxSize;
	
	GatherInfo **buffer;
};

