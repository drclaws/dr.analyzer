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
	
	try {
		this->dataTransport = new DataTransport();
	}
	catch (std::exception) {
		TerminateThread(waiterThread, 0);
		CloseHandle(waiterThread);
		FreeLibraryAndExitThread(libHModule, 0);
	}

	this->AddLoadedResToBuff();

	bool detoured = this->DetourFuncs();
	if (!detoured) {
		TerminateThread(waiterThread, 0);
		CloseHandle(waiterThread);
		this->AddToBuff(new GatherInfo(GatherType::GatherDetourError, GatherFuncType::GatherBufferSender));
		this->isDisconnecting = true;
		delete this->dataTransport;
		FreeLibraryAndExitThread(libHModule, 0);
	}

	while (true) {
		// Wait for buff input
		if (!this->isDisconnecting) {
			this->addCv.wait(addCVLock);
		}
		else if (detoured) {
			if (!this->UndetourFuncs()) {
				this->AddToBuff(new GatherInfo(GatherType::GatherDetourError, GatherFuncType::GatherBufferSender));
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
	delete this->dataTransport;
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
		DetourTransactionAbort();
		return false;
	}

	return true;
}

bool Gatherer::UndetourFuncs()
{
	if (DetourTransactionBegin() != NO_ERROR) {
		return false;
	}
	if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourDetach(&(PVOID&)OrigCreateFile2, NewCreateFile2) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigCreateFileA, NewCreateFileA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigCreateFileW, NewCreateFileW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigOpenFileById, NewOpenFileById) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourDetach(&(PVOID&)OrigLoadLibraryA, NewLoadLibraryA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigLoadLibraryW, NewLoadLibraryW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigLoadLibraryExA, NewLoadLibraryExA) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}
	if (DetourDetach(&(PVOID&)OrigLoadLibraryExW, NewLoadLibraryExW) != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	if (DetourTransactionCommit() != NO_ERROR) {
		DetourTransactionAbort();
		return false;
	}

	return true;
}
