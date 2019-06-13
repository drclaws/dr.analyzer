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

#include <iostream>


Gatherer* gatherer = NULL;

Gatherer::Gatherer()
{

}

Gatherer::~Gatherer()
{
	
}

void Gatherer::TransferThreadFunc() {
	std::unique_lock<std::mutex> addCVLock(this->addCvMutex);
	std::unique_lock<std::mutex> buffFullCVLock(this->buffFullCvMutex);

	this->dataTransport = new DataTransport();
	this->AddLoadedResToBuff();
	this->DetourFuncs();

	bool detoured = true;

	while (true) {
		// Wait for buff input
		if (!this->isDisconnecting) {
			this->addCv.wait(addCVLock);
		}
		else if (detoured) {
			this->UndetourFuncs();
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

		if (this->isDisconnecting && this->addUsingCount == 0 && !detoured) {
			break;
		}
	}

	delete this->dataTransport;
}

void Gatherer::AddToBuff(GatherInfo *info) {
	this->addUsingCount++;
	this->buffMutex.lock();
	if (this->isDisconnecting) {
		delete info;
		this->buffMutex.unlock();
		this->addUsingCount--;
		return;
	}

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



void Gatherer::AddLoadedResToBuff() {
	const DWORD modulesLength = 500;
	DWORD modulesSizeNeeded, modulesAmount;
	
	HMODULE hModules[modulesLength];
	BuffObject *buffObj = new BuffObject();
	GatherInfo* tmpInfo;

	if (EnumProcessModules(GetCurrentProcess(), hModules, sizeof(hModules), &modulesSizeNeeded)) {
		for (unsigned int i = 0; i < (modulesAmount = modulesSizeNeeded / sizeof(HMODULE)); i++) {
			tmpInfo = LibraryHmoduleToInfoObject(hModules[i], GatherFuncType::GatherFilesOnLoad);
			if (!buffObj->AddInfo(tmpInfo)) {
				this->dataTransport->SendData(buffObj);
				buffObj = new BuffObject();
				buffObj->AddInfo(tmpInfo);
			}
		}
	}

	if (buffObj->IsEmpty()) {
		delete buffObj;
	}
	else {
		this->dataTransport->SendData(buffObj);
	}
}

void Gatherer::DetourFuncs()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)OrigCreateFile2, NewCreateFile2);
	DetourAttach(&(PVOID&)OrigCreateFileA, NewCreateFileA);
	DetourAttach(&(PVOID&)OrigCreateFileW, NewCreateFileW);
	DetourAttach(&(PVOID&)OrigOpenFileById, NewOpenFileById);

	DetourAttach(&(PVOID&)OrigLoadLibraryA, NewLoadLibraryA);
	DetourAttach(&(PVOID&)OrigLoadLibraryW, NewLoadLibraryW);
	DetourAttach(&(PVOID&)OrigLoadLibraryExA, NewLoadLibraryExA);
	DetourAttach(&(PVOID&)OrigLoadLibraryExW, NewLoadLibraryExW);

	DetourTransactionCommit();
}

void Gatherer::UndetourFuncs()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	
	DetourDetach(&(PVOID&)OrigCreateFile2, NewCreateFile2);
	DetourDetach(&(PVOID&)OrigCreateFileA, NewCreateFileA);
	DetourDetach(&(PVOID&)OrigCreateFileW, NewCreateFileW);
	DetourDetach(&(PVOID&)OrigOpenFileById, NewOpenFileById);

	DetourDetach(&(PVOID&)OrigLoadLibraryA, NewLoadLibraryA);
	DetourDetach(&(PVOID&)OrigLoadLibraryW, NewLoadLibraryW);
	DetourDetach(&(PVOID&)OrigLoadLibraryExA, NewLoadLibraryExA);
	DetourDetach(&(PVOID&)OrigLoadLibraryExW, NewLoadLibraryExW);

	DetourTransactionCommit();
}
