#include "stdafx.h"
#include "Gatherer.h"

#include <Windows.h>
#include <psapi.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <string>

#include <detours.h>

#include "DataTransport.h"
#include "hook_funcs.h"
#include "flags.h"
#include "hook.h"


Gatherer* gatherer = NULL;

Gatherer::Gatherer()
{

}

Gatherer::~Gatherer()
{
	if (this->dataTransport != NULL) {
	    delete this->dataTransport;
	}
	if (this->buffObj != NULL) {
	    delete this->buffObj;
	}
}

void Gatherer::TransferThreadFunc() {
	std::unique_lock<std::mutex> addCVLock(this->addCvMutex);
	std::unique_lock<std::mutex> buffFullCVLock(this->buffFullCvMutex);
	
	try {
		this->dataTransport = new DataTransport();
	}
	catch (std::exception) {
		ReleaseSemaphore(waiterSemaphore, 1, NULL);
		return;
	}

	bool detoured = DetourFuncs();
	if (!detoured) {
		ReleaseSemaphore(waiterSemaphore, 1, NULL);
		this->AddToBuff(new GatherInfo(GatherType::GatherDetourError, GatherFuncType::GatherBufferSender));
		this->isDisconnecting = true;
		this->dataTransport->SendData(this->buffObj);
		this->buffObj = NULL;
		delete this->dataTransport;
		this->dataTransport = NULL;
		return;
	}
	std::thread getLoadedResThread(&Gatherer::AddLoadedResToBuff, this);

	while (true) {
		// Wait for buff input
		if (!this->isDisconnecting) {
			this->addCv.wait(addCVLock);
		}
		else if (detoured) {
			if (!UndetourFuncs()) {
				this->AddToBuff(new GatherInfo(GatherType::GatherUndetourError, GatherFuncType::GatherBufferSender));
			}
			detoured = false;
		}

		this->threadNotified = true;

		if (!this->isDisconnecting) {
			this->buffFullCv.wait_for(buffFullCVLock, std::chrono::milliseconds(200));
		}

		this->buffMutex.lock();

		if (this->buffObj != NULL) {
			this->dataTransport->SendData(this->buffObj);
			this->buffObj = NULL;
		}

		this->threadNotified = false;

		this->buffMutex.unlock();

		if (this->isDisconnecting && !detoured && this->addUsingCount == 0) {
			break;
		}
	}
	getLoadedResThread.join();
	delete this->dataTransport;
	this->dataTransport = NULL;
}

void Gatherer::AddToBuff(GatherInfo *info) {
	this->addUsingCount++;
	if (this->isDisconnecting) {
		delete info;
		this->addUsingCount--;
		return;
	}

	this->buffMutex.lock();

	if (this->buffObj == NULL) {
		this->buffObj = new BuffObject();
		this->buffObj->AddInfo(info);
	}
	else if (!this->buffObj->AddInfo(info)) {
		this->buffFullCv.notify_one();
		this->dataTransport->SendData(this->buffObj);
		this->buffObj = new BuffObject();
		this->buffObj->AddInfo(info);
	}

	if (!this->threadNotified) {
		this->addCv.notify_one();
	}

	this->buffMutex.unlock();
	this->addUsingCount--;
}

void Gatherer::SetDisconnect()
{
	this->isDisconnecting = true;
	this->addCv.notify_one();
}

void Gatherer::AddLoadedResToBuff() {
	DWORD modulesLength = 1023;
	DWORD modulesSizeNeeded, modulesAmount;
	
	HMODULE *hModules = new HMODULE[modulesLength];
	BuffObject *buffObj = new BuffObject();
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
				this->dataTransport->SendData(buffObj);
				buffObj = new BuffObject();
				buffObj->AddInfo(tmpInfo);
			}
		}
	}
	else {
	    buffObj->AddInfo(new GatherInfo(GatherType::GatherLibrariesOnLoadNotGathered, GatherFuncType::GatherFilesOnLoad));
	}
    delete[] hModules;
    
    SearchFileHandles(this->dataTransport, &buffObj);
    
	if (buffObj->IsEmpty()) {
		delete buffObj;
	}
	else {
		this->dataTransport->SendData(buffObj);
	}
}
