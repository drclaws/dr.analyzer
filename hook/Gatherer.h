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
	Gatherer();
	~Gatherer();

	void TransferThreadFunc();
	void AddToBuff(GatherInfo* info);
	void SetDisconnect();

private:
	std::atomic_bool isDisconnecting = false;

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

	bool DetourFuncs();
	bool UndetourFuncs();
};


extern Gatherer* gatherer;
