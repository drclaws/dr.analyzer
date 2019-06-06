#pragma once

#include "GatherInfo.h"

#include "flags.h"

const INT32 BUFF_MAX_LENGTH = 500;
const INT32 BUFF_MAX_SIZE = MAX_NAME_LENGTH * 2 + sizeof(gather_flag_t) * 2 + sizeof(INT32);


class BuffObject
{
public:
	const int maxLength = BUFF_MAX_LENGTH;
	const int maxSize = BUFF_MAX_SIZE;

	BuffObject();
	~BuffObject();

	bool AddInfo(GatherInfo* info);
	bool IsEmpty();
	void* ToMessage();

private:
	int length = 0;
	int size = 0;

	int remainsLength = maxLength;
	int remainsSize = maxSize;
	
	GatherInfo *buffer[BUFF_MAX_LENGTH];
};

