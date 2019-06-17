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

DWORD WINAPI WaiterForCloseFunc(LPVOID) {
	std::cout << " 8 " << std::endl;
	if ((gatherThread = CreateThread(NULL, 0, &GatherThreadFunc, NULL, 0, NULL)) == NULL) {
		std::cout << " 9 " << std::endl;
		FreeLibraryAndExitThread(libHModule, 1);
	}


	std::cout << " 10 " << std::endl;
	std::wstring waiterSemaphoreName = L"Global\\dr_analyzer_waiter_semaphore_" + std::to_wstring(GetCurrentProcessId());
	HANDLE waiterSemaphore = OpenSemaphoreW(
		SEMAPHORE_ALL_ACCESS,
		FALSE,
		waiterSemaphoreName.c_str()
	);

	if (waiterSemaphore != NULL) {
		std::cout << " 11 " << std::endl;
		WaitForSingleObject(waiterSemaphore, INFINITE);
		CloseHandle(waiterSemaphore);
	}
	else {
		std::cout << " 12 " << std::endl;
		gatherer->AddToBuff(new GatherInfo(GatherType::GatherWaiterError, GatherFuncType::GatherWaiter));
	}

	std::cout << " 13 " << std::endl;
	gatherer->isDisconnecting = true;
	if (gatherThread != NULL) {
		WaitForSingleObject(gatherThread, INFINITE);
		CloseHandle(gatherThread);
		gatherThread = NULL;
	}

	std::cout << " 14 " << std::endl;
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
		std::cout << " 1 " << std::endl;
		libHModule = hModule;
		if (!GetOrigAddresses()) {
			return FALSE;
		}
		std::cout << " 2 " << std::endl;
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)OrigExitProcess, NewExitProcess);
		if (DetourTransactionCommit() != NO_ERROR) {
			return FALSE;
		}
		std::cout << " 3 " << std::endl;
		gatherer = new Gatherer();
		std::cout << " 4 " << std::endl;
		if ((waiterThread = CreateThread(NULL, 0, &WaiterForCloseFunc, NULL, 0, NULL)) == NULL) {
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourDetach(&(PVOID&)OrigExitProcess, NewExitProcess);
			DetourTransactionCommit();
			return FALSE;
		}
		std::cout << "5" << std::endl;
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		std::cout << " 6 " << std::endl;
		delete gatherer;
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OrigExitProcess, NewExitProcess);
		DetourTransactionCommit();
		std::cout << " 7 " << std::endl;
	}

    return TRUE;
}
