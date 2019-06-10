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
	this->dataTransport = new DataTransport();
}

Gatherer::~Gatherer()
{
	this->buffMutex.lock();
	this->isDisconnecting = true;
	this->buffMutex.unlock();

	this->ToOrigFuncs();

	this->addCv.notify_one();
	this->buffMutex.lock();

	while (this->buffObj != NULL) {
		this->buffMutex.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		this->buffMutex.lock();
	}

	this->queueConnectionThread.join();

	delete this->dataTransport;
	this->buffMutex.unlock();
}

void Gatherer::Activate() {
	this->buffMutex.lock();

	this->dataTransport->ActivateSender();

	this->AddLoadedResToBuff();

	this->queueConnectionThread = std::thread(&Gatherer::TransferThreadFunc, this);

	this->buffMutex.unlock();

	if (!this->DetourFuncs()) {
		throw std::exception("Detour error");
	}
}

void Gatherer::AddToBuff(GatherInfo *info) {
	this->buffMutex.lock();
	if (this->isDisconnecting) {
		this->buffMutex.unlock();
		delete info;
		return;
	}

	if (this->buffObj == NULL) {
		this->buffObj = new BuffObject();
	}

	while (!this->buffObj->AddInfo(info)) {
		this->buffFull = true;
		this->buffFullCv.notify_one();

		this->buffMutex.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		this->buffMutex.lock();
		if (this->buffObj == NULL) {
			this->buffObj = new BuffObject();
		}
	}

	if (!this->threadNotified) {
		this->addCv.notify_one();
	}

	this->buffMutex.unlock();
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

void Gatherer::TransferThreadFunc() {
	std::unique_lock<std::mutex> addCVLock(this->addCvMutex);
	std::unique_lock<std::mutex> buffFullCVLock(this->buffFullCvMutex);

	std::chrono::high_resolution_clock::time_point waitStart;
	std::chrono::milliseconds duration;

	while (true) {
		waitStart = std::chrono::high_resolution_clock::now();

		// Wait for buff input 
		this->addCv.wait(addCVLock);
		this->threadNotified = true;

		duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - waitStart);

		if (!this->buffFull && duration.count() < 1000ll) {
			// Wait if package was 
			this->buffFullCv.wait_for(buffFullCVLock, std::chrono::milliseconds(1000) - duration);
		}

		this->buffMutex.lock();

		if (this->buffObj != NULL) {
			this->dataTransport->SendData(this->buffObj);
		}
		this->buffObj = NULL;
		this->threadNotified = false;
		
		this->buffMutex.unlock();

		if (this->isDisconnecting) {
			break;
		}
	}
}

bool Gatherer::DetourFuncs()
{
	if (DetourTransactionBegin() != NO_ERROR) {
		return false;
	}
	if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourAttach(&(PVOID&)OrigCreateFile2, NewCreateFile2) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigCreateFileA, NewCreateFileA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigCreateFileW, NewCreateFileW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigOpenFileById, NewOpenFileById) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourAttach(&(PVOID&)OrigLoadLibraryA, NewLoadLibraryA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigLoadLibraryW, NewLoadLibraryW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigLoadLibraryExA, NewLoadLibraryExA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigLoadLibraryExW, NewLoadLibraryExW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourTransactionCommit() != NO_ERROR) {
		return false;
	}

	return true;
}

bool Gatherer::ToOrigFuncs()
{
	if (DetourTransactionBegin() != NO_ERROR) {
		return false;
	}
	if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	
	if (DetourAttach(&(PVOID&)OrigCreateFile2, NewCreateFile2) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigCreateFileA, NewCreateFileA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigCreateFileW, NewCreateFileW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigOpenFileById, NewOpenFileById) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourAttach(&(PVOID&)OrigLoadLibraryA, NewLoadLibraryA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigLoadLibraryW, NewLoadLibraryW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigLoadLibraryExA, NewLoadLibraryExA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourAttach(&(PVOID&)OrigLoadLibraryExW, NewLoadLibraryExW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourTransactionCommit() != NO_ERROR) {
		return false;
	}

	return true;
}
