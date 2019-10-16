#pragma once
#include "stdafx.h"

#include "BuffObject.hpp"


// HMODULE of current library
extern HMODULE libHModule;

// HANDLE of waiter thread that waits semaphore
extern HANDLE waiterThread;

// HANDLE of waiter semaphore that indicates ending of info gathering
extern HANDLE waiterSemaphore;

// HANDLE of semaphore for indicates if necessary to free current library
extern HANDLE freeLibSemaphore;

// Searches and adds to buffer info about currently using files
void SearchFileHandles(BuffObject * &currBuff);

// Finds the ability to use certain types and functions from Native API
// by obtaining the build number of current Windows OS 
void GetFeaturesSupport();
