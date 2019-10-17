#include "stdafx.h"
#include "data_transportation.h"

#include <Windows.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <queue>
#include <string>

#include "hook_funcs.h"

#include "GatherInfo.hpp"
#include "BuffObject.hpp"
#include "flags.h"
#include "hook.h"


bool isDisconnecting = false;

HANDLE transportMapping = NULL;
PBYTE transportView = NULL;
HANDLE transportMutex = NULL;
HANDLE transportSemaphore = NULL;
HANDLE transportReceivedSemaphore = NULL;

std::thread senderThread;
std::mutex queueOperMutex;
std::condition_variable queueOperEndedCV;

std::queue<BuffObject*> buffQueue;


void SenderThreadFunc();


inline void CloseSharedMemory() {
	if (transportMapping != NULL) {
	    if (transportView != NULL) {
    		UnmapViewOfFile(transportView);
    		transportView = NULL;
    	}
    	else {
    	    CloseHandle(transportMapping);
            transportMapping = NULL;
            return;
    	}
		CloseHandle(transportMapping);
		transportMapping = NULL;
	}
	
	if (transportMutex == NULL) {
		return;
	}
	CloseHandle(transportMutex);
    transportMutex = NULL;
	
	if (transportSemaphore == NULL) {
	    return;
	}
	CloseHandle(transportSemaphore);
	transportSemaphore = NULL;
	
	if (transportReceivedSemaphore != NULL) {
	    CloseHandle(transportReceivedSemaphore);
	    transportReceivedSemaphore = NULL;
	}
}

const bool DataTransportInit()
{
	// Create connection
	DWORD pid = GetCurrentProcessId();
    
    std::wstring pidStr = std::to_wstring(pid);
    
	std::wstring mapping_loc = L"Global\\dr_analyzer_buffer_" + pidStr;
	std::wstring mutex_name = L"Global\\dr_analyzer_mutex_" + pidStr;
	std::wstring semaphore_sent_loc = L"Global\\dr_analyzer_sent_semaphore_" + pidStr;
    std::wstring semaphore_received_loc = L"Global\\dr_analyzer_received_semaphore_" + pidStr;
    
	transportMapping = OpenFileMappingW(
		FILE_MAP_WRITE,
		FALSE,
		mapping_loc.c_str()
	);

	if (transportMapping == NULL) {
		return false;
	}

	transportView = (PBYTE)MapViewOfFile(
		transportMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		BUFF_MAX_SIZE + sizeof(buff_size_t)
	);

	if (transportView == NULL) {
		CloseSharedMemory();
		return false;
	}

	transportMutex = OpenMutexW(
		MUTEX_ALL_ACCESS,
		FALSE,
		mutex_name.c_str()
	);

	if (transportMutex == NULL) {
		CloseSharedMemory();
		return false;
	}

	transportSemaphore = OpenSemaphoreW(
		SEMAPHORE_ALL_ACCESS,
		FALSE,
		semaphore_sent_loc.c_str()
	);
    
	if (transportSemaphore == NULL) {
		CloseSharedMemory();
		return false;
	}
    
    transportReceivedSemaphore = OpenSemaphoreW(
        SEMAPHORE_ALL_ACCESS,
        FALSE,
        semaphore_received_loc.c_str()
    );
    
    if (transportReceivedSemaphore == NULL) {
        CloseSharedMemory();
		return false;
    }
    
	// Launch sender's thread
	senderThread = std::thread(&SenderThreadFunc);
	
	BuffObject* buff = new BuffObject();
	buff->AddInfo(new GatherInfo(GatherType::GatherStarted, GatherFuncType::GatherConnection));
	SendBuff(buff);

	return true;
}


void DataTransportStop()
{
	queueOperMutex.lock();

	isDisconnecting = true;
	BuffObject* buff = new BuffObject();
	buff->AddInfo(new GatherInfo(GatherType::GatherStopped, GatherFuncType::GatherConnection));
	buffQueue.push(buff);
	queueOperEndedCV.notify_one();

	queueOperMutex.unlock();
	
	senderThread.join();

	CloseSharedMemory();
}

void SendBuff(BuffObject* info)
{
	std::unique_lock<std::mutex> operationLock(queueOperMutex);
	if (isDisconnecting) {
		return;
	}

	buffQueue.push(info);
	queueOperEndedCV.notify_one();
}


void SenderThreadFunc()
{
    std::mutex queueOperEndedMutex;
	std::unique_lock<std::mutex> cvLock(queueOperEndedMutex);

	DWORD waitRes = WAIT_OBJECT_0;

	while (true) {
	    const std::chrono::seconds waitTime(5);
		queueOperEndedCV.wait_for(cvLock, waitTime);
		queueOperMutex.lock();
		
		if (buffQueue.size() == 0) {
			BuffObject* buff = new BuffObject();
			buff->AddInfo(new GatherInfo(GatherType::GatherStillUp, GatherFuncType::GatherConnection));
			buffQueue.push(buff);
		}

		while (!buffQueue.empty()) {
			BuffObject* buff = buffQueue.front();
			buffQueue.pop();
			
			queueOperMutex.unlock();

			PBYTE message = buff->ToMessage();

			if((waitRes = WaitForSingleObject(transportMutex, 10000)) != WAIT_OBJECT_0)
			{
			    queueOperMutex.lock();
			    isDisconnecting = true;
			    delete buff;
			    delete[] message;
			    break;
			}

			std::memcpy(transportView, message, buff->MessageSize());

			ReleaseMutex(transportMutex);
			ReleaseSemaphore(transportSemaphore, 1, NULL);
			
			if((waitRes = WaitForSingleObject(transportReceivedSemaphore, 10000)) != WAIT_OBJECT_0) {
			    queueOperMutex.lock();
			    isDisconnecting = true;
			    delete buff;
			    delete[] message;
			    break;
			}
			
			delete buff;
			delete[] message;
			queueOperMutex.lock();
		}

		if (isDisconnecting) {
			queueOperMutex.unlock();
			break;
		}
        
		queueOperMutex.unlock();
	}
	
	if(waitRes != WAIT_OBJECT_0) {
	    ReleaseSemaphore(waiterSemaphore, 1, NULL);
	}
	while(!buffQueue.empty()) {
        delete buffQueue.front();
    	buffQueue.pop();
    }
}
