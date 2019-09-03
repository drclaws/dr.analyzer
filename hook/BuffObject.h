#pragma once
#include "stdafx.h"
#include "GatherInfo.h"

#include "flags.h"

const INT32 BUFF_MAX_LENGTH = 500;
const INT32 BUFF_MAX_SIZE = MAX_NAME_LENGTH * sizeof(WCHAR) + sizeof(gather_flag_t) * 2 + sizeof(INT32);


class BuffObject
{
public:
	const INT32 maxLength = BUFF_MAX_LENGTH;
	const buff_size_t maxSize = BUFF_MAX_SIZE;

	BuffObject();
	~BuffObject();

	bool AddInfo(GatherInfo* info);
	bool IsEmpty();
	PBYTE ToMessage();
	buff_size_t MessageSize();

private:
	INT32 length = 0;
	buff_size_t size = 0;

	INT32 remainsLength = BUFF_MAX_LENGTH;
	INT32 remainsSize = BUFF_MAX_SIZE;
	
	GatherInfo** buffer;
};
