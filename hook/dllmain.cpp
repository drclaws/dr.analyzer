// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "stdafx.h"

//#include "hook.h"
#include "hook_funcs.h"

#include <detours.h>
#include <iostream>

#include <atomic>

#include "Gatherer.h"
#include "hook.h"


HMODULE libModule;
int curr = 0;

DWORD WINAPI ActivateThreadFunc(LPVOID) {
	gatherer->TransferThreadFunc();
	ExitThread(0);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		libModule = hModule;
		if (!GetOrigAddresses()) {
			std::cout << "addresses not gotten" << std::endl;
			return FALSE;
		}
		gatherer = new Gatherer();
		if ((senderThread = CreateThread(NULL, 0, &ActivateThreadFunc, NULL, 0, NULL)) == NULL) {
			return FALSE;
		}
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)OrigExitProcess, NewExitProcess);
		DetourTransactionCommit();
		std::cout << "process attach" << std::endl;
		break;

	case DLL_THREAD_ATTACH:
		std::cout << "thread attach" << ++curr << std::endl;
		break;

	case DLL_THREAD_DETACH:
		std::cout << "thread detach" << --curr << std::endl;
		break;

	case DLL_PROCESS_DETACH:
		std::cout << "process detach" << std::endl;
		delete gatherer;
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OrigExitProcess, NewExitProcess);
		DetourTransactionCommit();
		break;

	default:
		break;
	}

    return TRUE;
}
