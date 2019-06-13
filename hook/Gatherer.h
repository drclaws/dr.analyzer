#pragma once

#include <Windows.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>


#include "BuffObject.h"
#include "DataTransport.h"
#include "GatherInfo.h"


class Gatherer
{
public:
	bool isDisconnecting = false;

	Gatherer();
	~Gatherer();

	void TransferThreadFunc();
	void AddToBuff(GatherInfo* info);

private:
	std::atomic_bool threadNotified = false;

	std::atomic_uint addUsingCount = 0;

	DataTransport* dataTransport = NULL;

	std::mutex buffMutex;

	std::condition_variable addCv;
	std::mutex addCvMutex;

	std::condition_variable buffFullCv;
	std::mutex buffFullCvMutex;

	BuffObject* buffObj = NULL;

	void AddLoadedResToBuff();

	void DetourFuncs();
	void UndetourFuncs();
};


extern Gatherer* gatherer;
