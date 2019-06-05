#include "stdafx.h"
#include "Gatherer.h"

#include <Windows.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <string>

#include <detours.h>

#include "DataTransport.h"
#include "hook_funcs.h"


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
	this->queueConnectionThread = new std::thread(this->TransferThreadFunc);

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
		std::this_thread::sleep_for(std::chrono::microseconds(10));
		uniqueLock.lock();
	}

	this->queueConnectionThread->join();
	delete this->queueConnectionThread;

	delete this->dataTransport;
	uniqueLock.unlock();

}


void Gatherer::AddFileToBuff(HANDLE fileHandle) {
	//LPCWSTR name;
	//INT32 nameLength;
	// TODO GetFileInfo


	this->AddToBuff(new GatherInfo(GatherType::GatherFileOpen, name, nameLength));
}

void Gatherer::AddLibToBuff(HANDLE libHandle) {
	//LPCWSTR name;
	//INT32 nameLength;
	// TODO GetLibInfo


	this->AddToBuff(new GatherInfo(GatherType::GatherLibraryOpen, name, nameLength));
}

void Gatherer::WarningNameToBig(INT8 fileType)
{
	this->AddToBuff(new GatherInfo(fileType, NULL, (INT32)-1));
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
	std::unique_lock<std::mutex> uniqueLock(this->buffMutex);
	uniqueLock.lock();

	if (this->isDisconnecting) {
		uniqueLock.unlock();
		return;
	}

	// TODO GetCurrentInfo

	uniqueLock.unlock();
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
		this->buffObj == NULL;
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
	if (DetourAttach(&(PVOID&)OrigOpenFile, NewOpenFile) != NO_ERROR) {
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
	if (DetourAttach(&(PVOID&)OrigOpenFile, NewOpenFile) != NO_ERROR) {
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
}
