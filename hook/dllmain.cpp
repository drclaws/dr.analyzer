#include "stdafx.h"

#include <detours.h>
#include <stdlib.h>
#include "hook_funcs.h"

#include "gathering.h"
#include "hook.h"


DWORD WINAPI GatherThreadFunc(LPVOID) {
	GatherThreadFunc();
	ExitThread(0);
}

inline void CloseWaiter(int errCode) {
    //Check if semaphore has already been released
	if (WaitForSingleObject(freeLibSemaphore, 0) == WAIT_OBJECT_0) {
	    //No, so waiter was called from out of the process.
	    //The library must be freed
		FreeLibraryAndExitThread(libHModule, errCode);
	}
	else {
        //Yes, so it means that detoured ExitProcess was called.
        //Only the thread must to be freed
		ExitThread(errCode);
	}
}

DWORD WINAPI WaiterForCloseFunc(LPVOID) {
	HANDLE gatherThread = CreateThread(NULL, 0, &GatherThreadFunc, NULL, 0, NULL);

	if (gatherThread == NULL) {
		CloseWaiter(1);
	}
	else {
		WaitForSingleObject(waiterSemaphore, INFINITE);

		SetDisconnect();

		WaitForSingleObject(gatherThread, INFINITE);
		CloseHandle(gatherThread);
		CloseWaiter(0);
	}
}

inline void CloseSemaphores() {
	CloseHandle(waiterSemaphore);
	CloseHandle(freeLibSemaphore);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (DetourIsHelperProcess()) {
	    // Means that this is a helper process for loading library in process of another architecture.
	    // Will be helpful when it will be implemented multiple architecture of process info gathering
		return TRUE;
	}

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		libHModule = hModule;
		
		GetFeaturesSupport();
		
		if (!GetOrigAddresses()) {
			return FALSE;
		}
        
        // Because it's possible in DllMain that CRT or STL library
        // hasn't been loaded yet, only WinAPI from Kernel32 and Native API can be used.
        // Therefore, it's necessary to use this construction to generate the semaphore name by pid.
        #ifndef SEM_NAME_TEMP
        #define SEM_NAME_TEMP L"Global\\dr_analyzer_waiter_semaphore_"
        
        const size_t pid_start = sizeof(SEM_NAME_TEMP) / sizeof(wchar_t) - 1;
        const size_t ul_dec_max_size = 11,
            arr_size = sizeof(SEM_NAME_TEMP) / sizeof(wchar_t) + ul_dec_max_size;
        
	    wchar_t semaphore_path[arr_size] = SEM_NAME_TEMP;
	    
	    #undef SEM_NAME_TEMP
	    #else
	    #error Macros SEM_NAME_TEMP is already defined somewhere else 
	    #endif //SEM_NAME_TEMP
	    
	    if (_ultow_s(
                GetCurrentProcessId(),
				&semaphore_path[pid_start],
                ul_dec_max_size + 1,
                10) != NULL) {
              return FALSE;      
        }
		if ((waiterSemaphore = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, FALSE, semaphore_path)) == NULL) {
			return FALSE;
		}

		if ((freeLibSemaphore = CreateSemaphoreW(NULL, 1, 1, NULL)) == NULL) {
			CloseHandle(waiterSemaphore);
			return FALSE;
		}

		if (!DetourExitProcess()) {
			CloseSemaphores();
			return FALSE;
		}
		
		if ((waiterThread = CreateThread(NULL, 0, &WaiterForCloseFunc, NULL, 0, NULL)) == NULL) {
			UndetourExitProcess();
			CloseSemaphores();
			return FALSE;
		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH && !DetourIsHelperProcess()) {
		UndetourExitProcess();
		CloseSemaphores();
	}

    return TRUE;
}
