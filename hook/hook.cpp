#include "stdafx.h"
#include <winternl.h>
#include "hook.h"

#include <exception>

#include "DataTransport.h"
#include "GatherInfo.h"
#include "flags.h"


#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004
#define STATUS_BUFFER_OVERFLOW 0x80000005
#define STATUS_BUFFER_TOO_SMALL 0xC0000023

#define ProcessHandleInformation 51
#define ObjectTypesInformation 3

#define MAX_CONTAINER_SIZE 0x800000


typedef NTSTATUS (NTAPI *_NtQueryInformationProcess)(
    HANDLE ProcessHandle,
    ULONG ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
    );
typedef NTSTATUS (NTAPI *_NtQueryObject)(
    HANDLE ObjectHandle,
    ULONG ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG ObjectInformationLength,
    PULONG ReturnLength
    );
typedef NTSYSAPI BOOLEAN (NTAPI *_RtlEqualUnicodeString)(
    const UNICODE_STRING *String1,
    const UNICODE_STRING *String2,
    BOOLEAN CaseInSensitive
    );
typedef NTSYSAPI VOID (NTAPI *_RtlInitUnicodeString)(
    PUNICODE_STRING DestinationString,
    __drv_aliasesMem PCWSTR SourceString
    );

typedef struct _PROCESS_HANDLE_TABLE_ENTRY_INFO
{
    HANDLE Handle;
        ULONG_PTR HandleCount;
        ULONG_PTR PointerCount;
        ULONG GrantedAccess;
        ULONG TypeIndex;
        ULONG HandleAttributes;
        ULONG Reserved;
} PROCESS_HANDLE_TABLE_ENTRY_INFO, *PPROCESS_HANDLE_TABLE_ENTRY_INFO;

typedef struct _PROCESS_HANDLE_SNAPSHOT_INFORMATION
{
    ULONG_PTR LengthPtr;
    ULONG_PTR Reserved;
    PROCESS_HANDLE_TABLE_ENTRY_INFO Handles[1];
} PROCESS_HANDLE_SNAPSHOT_INFORMATION, *PPROCESS_HANDLE_SNAPSHOT_INFORMATION;

typedef struct _OBJECT_TYPE_INFORMATION
{
    UNICODE_STRING TypeName;
    BYTE Reserved[sizeof(LONG) * 17 + sizeof(BOOLEAN) + sizeof(USHORT) + sizeof(GENERIC_MAPPING)];
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_TYPES_INFORMATION
{
    ULONG Length;
    OBJECT_TYPE_INFORMATION Types[1];
} OBJECT_TYPES_INFORMATION, *POBJECT_TYPES_INFORMATION;


HMODULE libHModule = NULL;

HANDLE waiterThread = NULL;
HANDLE waiterSemaphore = NULL;

HANDLE freeLibSemaphore = NULL;


inline void SendToDT(DataTransport *dataTransport, BuffObject **buffObj, GatherInfo *info) {
    if (!(*buffObj)->AddInfo(info)) {
        dataTransport->SendData(*buffObj);
    	*buffObj = new BuffObject();
    	(*buffObj)->AddInfo(info);
    }
}

ULONG GetFileHandleTypeNumber() {
    HMODULE ntdllHandle = GetModuleHandleA("ntdll.dll");
    _NtQueryObject NtQueryObject =
            (_NtQueryObject)GetProcAddress(ntdllHandle, "NtQueryObject");
    _RtlEqualUnicodeString RtlEqualUnicodeString =
            (_RtlEqualUnicodeString)GetProcAddress(ntdllHandle, "RtlEqualUnicodeString");
    _RtlInitUnicodeString RtlInitUnicodeString = 
            (_RtlInitUnicodeString)GetProcAddress(ntdllHandle, "RtlInitUnicodeString");
    if(NtQueryObject == NULL) {
        throw std::exception();//L"Can't get \"NtQueryObject\" function address");
    }
    if(RtlEqualUnicodeString == NULL) {
        throw std::exception();//L"Can't get \"RtlEqualUnicodeString\" function address");
    }
    if(RtlInitUnicodeString == NULL) {
        throw std::exception();//L"Can't get \"RtlInitUnicodeString\" function address");
    }
    
    UNICODE_STRING searchedTypeName;
    RtlInitUnicodeString(&searchedTypeName, L"File");
    
    NTSTATUS status;
    ULONG returnedLength = 0;
    ULONG typesSize = 0x10000;
    POBJECT_TYPES_INFORMATION types = (POBJECT_TYPES_INFORMATION)std::malloc(typesSize);

    while ((status = NtQueryObject(
        NULL,
        ObjectTypesInformation,
        types,
        typesSize,
        &returnedLength
        )) == STATUS_INFO_LENGTH_MISMATCH || status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
    {
        std::free(types);
        if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) {
            typesSize = returnedLength;
        }
        else if ((typesSize *= 2) > MAX_CONTAINER_SIZE) {
            // TODO large array, imagine something
            throw std::exception();//L"");
        }
        
        types = (POBJECT_TYPES_INFORMATION)std::malloc(typesSize);
    }

    if(!NT_SUCCESS(status)) {
        std::free(types);
        // TODO error on getting array, imagine something
        throw std::exception();//L"Error on \"\"");
    }
    
    POBJECT_TYPE_INFORMATION typeInfo = &types->Types[0];
    for(ULONG i = 0; i < types->Length; i++) {
        if(RtlEqualUnicodeString(&searchedTypeName, &typeInfo->TypeName, TRUE)) {
            std::free(types);
            return i + 2;
        }
        typeInfo = (POBJECT_TYPE_INFORMATION)((PCHAR)(typeInfo + 1) + ((ULONG)(((ULONG)(typeInfo->TypeName.MaximumLength) + sizeof(ULONG_PTR) - 1)) & ~(sizeof(ULONG_PTR) - 1)));
    }
    
    std::free(types);
    // TODO file type not found, imagine something
    throw std::exception();//L"");
}

void SearchFileHandles(DataTransport *dataTransport, BuffObject **currBuff) {
    _NtQueryInformationProcess NtQueryInformationProcess =
        (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");

    if (NtQueryInformationProcess == NULL) {
        // TODO send warning "Can't get \"NtQueryObject\" function address"
        return;
    }

    ULONG fileHandleTypeNumber;

    try {
        fileHandleTypeNumber = GetFileHandleTypeNumber();
    }
    catch (const std::exception e) {
        // TODO send warning "Can't get opened files" with message from exception
        return;
    }
    
    NTSTATUS status;
    ULONG returnedLength;
    HANDLE handle = GetCurrentProcess();
    ULONG handlesInfoSize = 0x4000;
    PPROCESS_HANDLE_SNAPSHOT_INFORMATION handlesInfo = (PPROCESS_HANDLE_SNAPSHOT_INFORMATION)std::malloc(handlesInfoSize);

    while ((status = NtQueryInformationProcess(
        handle,
        ProcessHandleInformation,
        handlesInfo,
        handlesInfoSize,
        &returnedLength)) == STATUS_INFO_LENGTH_MISMATCH || status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
    {
        std::free(handlesInfo);
        if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) {
        
        }
        else {
            if ((handlesInfoSize *= 2) > MAX_CONTAINER_SIZE) {
                // TODO send warning "Can't get opened files" because of large handles amount
                return;
            }
            handlesInfo = (PPROCESS_HANDLE_SNAPSHOT_INFORMATION)std::malloc(handlesInfoSize);
        }
    }
    
    if (!NT_SUCCESS(status)) {
        // TODO send warning "Can't get opened files"
        std::free(handlesInfo);
        return;
    }
    
    PPROCESS_HANDLE_TABLE_ENTRY_INFO handleInfo = &handlesInfo->Handles[0];
    
    for(ULONG i = 0; i < handlesInfo->LengthPtr; i++) {
        PPROCESS_HANDLE_TABLE_ENTRY_INFO handleInfo = &handlesInfo->Handles[i];
        if(handleInfo->TypeIndex == fileHandleTypeNumber) {
            GatherInfo *tmpInfo = FileHandleToInfoObject(handleInfo->Handle, GatherFuncType::GatherFilesOnLoad);
            if (tmpInfo != NULL) {
                SendToDT(dataTransport, currBuff, tmpInfo);
            }
        }
    }

    std::free(handlesInfo);
}
