// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "stdafx.h"

#include <detours.h>
#include <string>
#include <iostream>
#include "hook_funcs.h"

#include "Gatherer.h"
#include "GatherInfo.h"
#include "flags.h"
#include "hook.h"


DWORD WINAPI GatherThreadFunc(LPVOID) {
	gatherer->TransferThreadFunc();
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

		gatherer->SetDisconnect();

		WaitForSingleObject(gatherThread, INFINITE);
		CloseHandle(gatherThread);
		CloseWaiter(0);
	}
}

inline void UndetourExitProcess() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)OrigExitProcess, NewExitProcess);
	DetourTransactionCommit();
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
		if (!GetOrigAddresses()) {
			return FALSE;
		}

		std::wstring waiterSemaphoreName = L"Global\\dr_analyzer_waiter_semaphore_" + std::to_wstring(GetCurrentProcessId());
		if ((waiterSemaphore = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, FALSE, waiterSemaphoreName.c_str())) == NULL) {
			return FALSE;
		}

		if ((freeLibSemaphore = CreateSemaphoreW(NULL, 1, 1, NULL)) == NULL) {
			CloseHandle(waiterSemaphore);
			return FALSE;
		}

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)OrigExitProcess, NewExitProcess);
		if (DetourTransactionCommit() != NO_ERROR) {
			CloseSemaphores();
			return FALSE;
		}
		gatherer = new Gatherer();
		if ((waiterThread = CreateThread(NULL, 0, &WaiterForCloseFunc, NULL, 0, NULL)) == NULL) {
			delete gatherer;
			UndetourExitProcess();
			CloseSemaphores();
			return FALSE;
		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		delete gatherer;
		UndetourExitProcess();
		CloseSemaphores();
	}

    return TRUE;
}
