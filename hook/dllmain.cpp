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
	if (WaitForSingleObject(freeLibSemaphore, 0) == WAIT_OBJECT_0) {
		FreeLibraryAndExitThread(libHModule, errCode);
	}
	else {
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
		return TRUE;
	}

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		libHModule = hModule;
		
		GetFeaturesSupport();
		
		if (!GetOrigAddresses()) {
			return FALSE;
		}

	    wchar_t semaphore_path[47] = L"Global\\dr_analyzer_waiter_semaphore_";
	    const ULONG_PTR pid_start = 36 * sizeof(wchar_t);
	    const int ul_dec_max_size = 10;
	    if (_ultow_s(
                GetCurrentProcessId(),
                (wchar_t*)((ULONG_PTR)semaphore_path + pid_start),
                ul_dec_max_size,
                ul_dec_max_size) != NULL) {
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
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		UndetourExitProcess();
		CloseSemaphores();
	}

    return TRUE;
}
