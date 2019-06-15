// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "stdafx.h"

#include <detours.h>
#include <string>

#include "hook_funcs.h"

#include "Gatherer.h"
#include "hook.h"

HMODULE libHModule;

DWORD WINAPI GatherThreadFunc(LPVOID) {
	gatherer->TransferThreadFunc();
	ExitThread(0);
}

DWORD WINAPI WaiterForCloseFunc(LPVOID) {
	if ((gatherThread = CreateThread(NULL, 0, &GatherThreadFunc, NULL, 0, NULL)) == NULL) {
		FreeLibraryAndExitThread(libHModule, 1);
	}

	std::wstring waiterSemaphoreName = L"Global\\dr_analyzer_waiter_semaphore_" + std::to_wstring(GetCurrentProcessId());
	HANDLE waiterSemaphore = OpenSemaphoreW(
		SEMAPHORE_ALL_ACCESS,
		FALSE,
		waiterSemaphoreName.c_str()
	);

	WaitForSingleObject(waiterSemaphore, INFINITE);

	gatherer->isDisconnecting = true;
	if (gatherThread != NULL) {
		WaitForSingleObject(gatherThread, INFINITE);
		gatherThread = NULL;
	}
	CloseHandle(waiterSemaphore);
	waiterThread = NULL;
	FreeLibraryAndExitThread(libHModule, 0);
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
		gatherer = new Gatherer();
		if ((waiterThread = CreateThread(NULL, 0, &WaiterForCloseFunc, NULL, 0, NULL)) == NULL) {
			return FALSE;
		}
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)OrigExitProcess, NewExitProcess);
		DetourTransactionCommit();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		delete gatherer;
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OrigExitProcess, NewExitProcess);
		DetourTransactionCommit();
	}

    return TRUE;
}
