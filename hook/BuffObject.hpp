#pragma once
#include "stdafx.h"
#include "GatherInfo.hpp"

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

	const bool AddInfo(GatherInfo* info);
	const bool IsEmpty() const;
	PBYTE ToMessage() const;
	const buff_size_t MessageSize() const;

private:
	INT32 length = 0;
	buff_size_t size = 0;
	
	GatherInfo* buffer[BUFF_MAX_LENGTH];
};
