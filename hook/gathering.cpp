#include "stdafx.h"
#include "gathering.h"

#include <Windows.h>
#include <psapi.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>

#include <string>

#include <detours.h>

#include "GatherInfo.hpp"
#include "BuffObject.hpp"
#include "data_transportation.h"
#include "hook_funcs.h"
#include "flags.h"
#include "hook.h"


std::atomic_bool isDisconnecting = false;
std::atomic_bool threadNotified = false;
std::atomic_uint addUsingCount = 0;

std::mutex buffMutex;
std::condition_variable addCv;

BuffObject* buffObj = NULL;

void AddLoadedResToBuff();


void GatherThreadFunc() {
    std::mutex addCvMutex;
	std::unique_lock<std::mutex> addCVLock(addCvMutex);
	
	if (!DataTransportInit()) {
	    ReleaseSemaphore(waiterSemaphore, 1, NULL);
        return;
	}

	bool detoured = DetourFuncs();
	if (!detoured) {
		ReleaseSemaphore(waiterSemaphore, 1, NULL);
		AddToBuff(new GatherInfo(GatherType::GatherDetourError, GatherFuncType::GatherBufferSender));
		isDisconnecting = true;
		SendBuff(buffObj);
		buffObj = NULL;
		DataTransportStop();
		return;
	}
	std::thread getLoadedResThread(&AddLoadedResToBuff);

	while (true) {
		// Wait for buff input
		if (!isDisconnecting) {
			addCv.wait(addCVLock);
		    threadNotified = true;
		}
		else if (detoured) {
			if (!UndetourFuncs()) {
				AddToBuff(new GatherInfo(GatherType::GatherUndetourError, GatherFuncType::GatherBufferSender));
			}
			detoured = false;
		}

		buffMutex.lock();

		if (buffObj != NULL) {
			SendBuff(buffObj);
			buffObj = NULL;
		}
		
        if (!isDisconnecting) {
		    threadNotified = false;
        }
		buffMutex.unlock();

		if (isDisconnecting && !detoured && addUsingCount == 0) {
			break;
		}
	}
	getLoadedResThread.join();
	DataTransportStop();
	if (buffObj != NULL) {
    	delete buffObj;
    	buffObj = NULL;
    }
}

void AddToBuff(GatherInfo *info) {
	if (info == nullptr)
		return; // TODO Remove after implementing all messages

	addUsingCount++;
	if (isDisconnecting) {
		delete info;
		addUsingCount--;
		return;
	}

	buffMutex.lock();

	if (buffObj == NULL) {
		buffObj = new BuffObject();
		buffObj->AddInfo(info);
		buffMutex.unlock();
	}
	else if (buffObj->AddInfo(info)) {
	    buffMutex.unlock();
	}
	else {
	    BuffObject* fullBuffObj = buffObj;
	    
		buffObj = new BuffObject();
		buffObj->AddInfo(info);
	    buffMutex.unlock();
	    
		SendBuff(fullBuffObj);
	}

	if (!threadNotified) {
		addCv.notify_one();
	}

	addUsingCount--;
}

void SetDisconnect()
{
	isDisconnecting = true;
	addCv.notify_one();
}

void AddLoadedResToBuff() {
	DWORD modulesLength = 1023;
	DWORD modulesSizeNeeded, modulesAmount;
	
	auto hModules = new HMODULE[modulesLength];
	auto buffObj = new BuffObject();
	GatherInfo* tmpInfo;
	
	BOOL status;
	
    while((status = EnumProcessModules(GetCurrentProcess(), hModules, sizeof(HMODULE) * modulesLength, &modulesSizeNeeded)) == TRUE
        && modulesSizeNeeded > (sizeof(HMODULE) * modulesLength))
    {
        delete[] hModules;
        modulesLength = modulesSizeNeeded / sizeof(HMODULE);
        hModules = new HMODULE[modulesLength];
    }
	if (status == TRUE) {
		for (unsigned int i = 0; i < (modulesAmount = modulesSizeNeeded / sizeof(HMODULE)); i++) {
			tmpInfo = LibraryHmoduleToInfoObject(hModules[i], GatherFuncType::GatherFilesOnLoad);
			if (tmpInfo == NULL) {
			    continue;
			}
			else if (!buffObj->AddInfo(tmpInfo)) {
				SendBuff(buffObj);
				buffObj = new BuffObject();
				buffObj->AddInfo(tmpInfo);
			}
		}
	}
	else {
	    buffObj->AddInfo(new GatherInfo(GatherType::GatherLibrariesOnLoadNotGathered, GatherFuncType::GatherFilesOnLoad));
	}
    delete[] hModules;
    
    SearchFileHandles(buffObj);
    
	if (buffObj->IsEmpty()) {
		delete buffObj;
	}
	else {
		SendBuff(buffObj);
	}
}
