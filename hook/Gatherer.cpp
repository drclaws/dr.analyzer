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


Gatherer* gatherer = NULL;


Gatherer::Gatherer()
{
	this->dataTransport = new DataTransport();

	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

	if (!this->DetourFuncs()) {
		delete this->dataTransport;
		throw std::exception("Detour error");
	}

	this->dataTransport->ActivateSender();

	this->AddLoadedResToBuff();

	this->queueConnectionThread = new std::thread(&Gatherer::TransferThreadFunc, this);

	uniqueLock.unlock();
}

Gatherer::~Gatherer()
{
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);

	uniqueLock.lock();
	this->isDisconnecting = true;
	uniqueLock.unlock();

	this->ToOrigFuncs();

	this->addCv.notify_one();
	uniqueLock.lock();

	while (this->buffObj != NULL) {
		uniqueLock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		uniqueLock.lock();
	}

	this->queueConnectionThread->join();
	delete this->queueConnectionThread;

	delete this->dataTransport;
	uniqueLock.unlock();
}

void Gatherer::AddToBuff(GatherInfo *info) {
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

	if (this->isDisconnecting) {
		uniqueLock.unlock();
		delete info;
		return;
	}

	if (this->buffObj == NULL) {
		this->buffObj = new BuffObject();
	}

	while (!this->buffObj->AddInfo(info)) {
		this->buffFull = true;
		this->buffFullCv.notify_one();

		uniqueLock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		uniqueLock.lock();
		if (this->buffObj == NULL) {
			this->buffObj = new BuffObject();
		}
	}

	if (!this->threadNotified) {
		this->addCv.notify_one();
	}

	uniqueLock.unlock();
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

#ifdef DR_HOOK_TEST
	else {
		std::cout << "Cannot acquire libs on load" << std::endl;
	}
#endif

	if (buffObj->IsEmpty()) {
		delete buffObj;
	}
	else {
		this->dataTransport->SendData(buffObj);
	}
}

void Gatherer::TransferThreadFunc() {
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
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

		uniqueLock.lock();

		if (this->buffObj != NULL) {
			this->dataTransport->SendData(this->buffObj);
		}
		this->buffObj = NULL;
		this->threadNotified = false;
		
		uniqueLock.unlock();

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
