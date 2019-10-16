#include "stdafx.h"
#include <winternl.h>
#include "hook.h"

#include <cstdlib>
#include <string>

#include "data_transportation.h"
#include "GatherInfo.hpp"
#include "flags.h"


#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004
#define STATUS_BUFFER_OVERFLOW 0x80000005
#define STATUS_BUFFER_TOO_SMALL 0xC0000023

#define SystemExtendedHandleInformation 64
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
typedef NTSTATUS (NTAPI *_NtQuerySystemInformation)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
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
typedef NTSYSAPI VOID (NTAPI *_RtlGetNtVersionNumbers)(
    PULONG NtMajorVersion,
    PULONG NtMinorVersion,
    PULONG NtBuildNumber
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
    _Out_opt_ ULONG_PTR Length;
    _Out_opt_ ULONG_PTR Unused;
    PROCESS_HANDLE_TABLE_ENTRY_INFO Handles[1];
} PROCESS_HANDLE_SNAPSHOT_INFORMATION, *PPROCESS_HANDLE_SNAPSHOT_INFORMATION;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX
{
    PVOID Object;
    ULONG_PTR Pid;
    ULONG_PTR Handle;
    ULONG GrantedAccess;
    USHORT CreatorBackTraceIndex;
    USHORT TypeIndex;
    ULONG HandleAttributes;
    ULONG Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX
{
    ULONG_PTR Length;
    ULONG_PTR Reserved;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
} SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;

typedef struct _OBJECT_TYPE_INFORMATION
{
    UNICODE_STRING TypeName;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG TotalPagedPoolUsage;
    ULONG TotalNonPagedPoolUsage;
    ULONG TotalNamePoolUsage;
    ULONG TotalHandleTableUsage;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    ULONG HighWaterPagedPoolUsage;
    ULONG HighWaterNonPagedPoolUsage;
    ULONG HighWaterNamePoolUsage;
    ULONG HighWaterHandleTableUsage;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    BOOLEAN SecurityRequired;
    BOOLEAN MaintainHandleCount;
    UCHAR TypeIndex;
    CHAR ReservedByte;
    ULONG PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
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

bool isSupportInfoProc = false;
bool isSupportTypeIndexInHandleInfoProc = false;

// String for error message that will be added to info buff
std::wstring lastErrorInfo = L"";

bool GetFileHandleTypeNumber(UCHAR &returnTypeIndex) {
    HMODULE ntdllHandle = GetModuleHandleA("ntdll.dll");
    _NtQueryObject NtQueryObject =
            (_NtQueryObject)GetProcAddress(ntdllHandle, "NtQueryObject");
    _RtlEqualUnicodeString RtlEqualUnicodeString =
            (_RtlEqualUnicodeString)GetProcAddress(ntdllHandle, "RtlEqualUnicodeString");
    _RtlInitUnicodeString RtlInitUnicodeString = 
            (_RtlInitUnicodeString)GetProcAddress(ntdllHandle, "RtlInitUnicodeString");
    if(NtQueryObject == NULL) {
        lastErrorInfo = L"Can't find \"NtQueryObject\" function";
        return false;
    }
    if(RtlEqualUnicodeString == NULL) {
        lastErrorInfo = L"Can't find \"RtlEqualUnicodeString\" function";
        return false;
    }
    if(RtlInitUnicodeString == NULL) {
        lastErrorInfo = L"Can't find \"RtlInitUnicodeString\" function";
        return false;
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
        
        if ((status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) && typesSize != returnedLength) {
            typesSize = returnedLength;
        }
        else if ((typesSize *= 2) > MAX_CONTAINER_SIZE) {
            lastErrorInfo = L"Too many handle types";
            return false;
        }
        
        types = (POBJECT_TYPES_INFORMATION)std::malloc(typesSize);
    }

    if(!NT_SUCCESS(status)) {
        std::free(types);
        lastErrorInfo = L"Unknown error on getting handle types list";
        return false;
    }
    ULONG length = types->Length;
    bool moreThanCanContain = false;
    if(types->Length > (ULONG)UCHAR_MAX - 2) {
        length = (ULONG)UCHAR_MAX - 2;
        moreThanCanContain = true;
    }
    
    POBJECT_TYPE_INFORMATION typeInfo = &types->Types[0];
    for(ULONG i = 0; i < length; i++) {
        if(RtlEqualUnicodeString(&searchedTypeName, &typeInfo->TypeName, TRUE)) {
            std::free(types);
            if (isSupportTypeIndexInHandleInfoProc) {
                returnTypeIndex = (UCHAR)typeInfo->TypeIndex;
            }
            else {
                returnTypeIndex = (UCHAR)(i + 2);
            }
            return true;
        }
        // Just google this expression if you don't understand
        typeInfo = (POBJECT_TYPE_INFORMATION)(
            (ULONG_PTR)typeInfo + (ULONG_PTR)(
                sizeof(OBJECT_TYPE_INFORMATION) + (
                    (
                        (ULONG_PTR)typeInfo->TypeName.MaximumLength + sizeof(ULONG_PTR) - 1
                    ) & ~(sizeof(ULONG_PTR) - 1))));
    }
    
    std::free(types);
    lastErrorInfo = L"Index of handle type for \"File\" not found";
    if (moreThanCanContain) {
        lastErrorInfo += L" in first " + std::to_wstring(length) + L" found types";
    }
    return false;
}

inline void SendToDT(BuffObject * &buffObj, GatherInfo *info) {
    if (!buffObj->AddInfo(info)) {
        SendBuff(buffObj);
    	buffObj = new BuffObject();
    	buffObj->AddInfo(info);
    }
}

inline PWSTR ErrorInfoInPWSTR() {
    size_t size = sizeof(WCHAR) * (lastErrorInfo.length() + 1);
    PWSTR message = (PWSTR)std::malloc(size);
    wcscpy_s(message, size, lastErrorInfo.c_str());
    return message;
}

void SearchFileHandlesModern(BuffObject * &currBuff) {
    _NtQueryInformationProcess NtQueryInformationProcess =
        (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");

    if (NtQueryInformationProcess == NULL) {
        lastErrorInfo = L"Can't find \"NtQueryInformationProcess\" function";
        SendToDT(
                 currBuff,
                 new GatherInfo(GatherType::GatherFilesOnLoadNotGathered, GatherFuncType::GatherFilesOnLoad, ErrorInfoInPWSTR(), (UINT32)lastErrorInfo.length()));
        return;
    }

    UCHAR fileHandleTypeNumber;
    if(!GetFileHandleTypeNumber(fileHandleTypeNumber)) {
        SendToDT(
                 currBuff,
                 new GatherInfo(GatherType::GatherFilesOnLoadNotGathered, GatherFuncType::GatherFilesOnLoad, ErrorInfoInPWSTR(), (UINT32)lastErrorInfo.length()));
        return;
    }
    
    NTSTATUS status;
    ULONG returnedLength = 0;
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
        if ((status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) && handlesInfoSize != returnedLength) {
            handlesInfoSize = returnedLength;
        }
        else {
            handlesInfoSize *= 2;
        }
        if (handlesInfoSize > MAX_CONTAINER_SIZE) {
            lastErrorInfo = L"Process has too many handles";
            SendToDT(
                currBuff,
                new GatherInfo(GatherType::GatherFilesOnLoadNotGathered, GatherFuncType::GatherFilesOnLoad, ErrorInfoInPWSTR(), (UINT32)lastErrorInfo.length()));
            return;
        }
        handlesInfo = (PPROCESS_HANDLE_SNAPSHOT_INFORMATION)std::malloc(handlesInfoSize);
    }
    
    if (!NT_SUCCESS(status)) {
        SendToDT(
            currBuff,
            new GatherInfo(GatherType::GatherFilesOnLoadNotGathered, GatherFuncType::GatherFilesOnLoad));
        std::free(handlesInfo);
        return;
    }
    
    for(ULONG i = 0; i < handlesInfo->Length; i++) {
        PPROCESS_HANDLE_TABLE_ENTRY_INFO handleInfo = &handlesInfo->Handles[i];
        if(handleInfo->TypeIndex == fileHandleTypeNumber) {
            GatherInfo *tmpInfo = FileHandleToInfoObject(handleInfo->Handle, GatherFuncType::GatherFilesOnLoad);
            if (tmpInfo != NULL) {
                SendToDT(currBuff, tmpInfo);
            }
        }
    }

    std::free(handlesInfo);
}

void SearchFileHandlesLegacy(BuffObject * &currBuff) {
    _NtQuerySystemInformation NtQuerySystemInformation = 
        (_NtQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation");
    if (NtQuerySystemInformation == NULL) {
        lastErrorInfo = L"Can't find \"NtQuerySystemInformation\" function";
        SendToDT(
                 currBuff,
                 new GatherInfo(GatherType::GatherFilesOnLoadNotGathered, GatherFuncType::GatherFilesOnLoad, ErrorInfoInPWSTR(), (UINT32)lastErrorInfo.length()));
        return;
    }
    UCHAR fileHandleTypeNumber;
    if(!GetFileHandleTypeNumber(fileHandleTypeNumber)) {
        SendToDT(
                 currBuff,
                 new GatherInfo(GatherType::GatherFilesOnLoadNotGathered, GatherFuncType::GatherFilesOnLoad, ErrorInfoInPWSTR(), (UINT32)lastErrorInfo.length()));
        return;
    }
    
    NTSTATUS status;
    ULONG returnedLength = 0;
    ULONG handlesInfoSize = 0x10000;
    PSYSTEM_HANDLE_INFORMATION_EX handlesInfo = (PSYSTEM_HANDLE_INFORMATION_EX)std::malloc(handlesInfoSize);
    
    while((status = NtQuerySystemInformation(
        SystemExtendedHandleInformation,
        handlesInfo,
        handlesInfoSize,
        &returnedLength)) == STATUS_INFO_LENGTH_MISMATCH || status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
    {
        std::free(handlesInfo);
        if ((status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) && handlesInfoSize != returnedLength) {
            handlesInfoSize = returnedLength;
        }
        else {
            handlesInfoSize *= 2;
        }
        if (handlesInfoSize > MAX_CONTAINER_SIZE) {
            lastErrorInfo = L"System has too many handles";
            SendToDT(
                currBuff,
                new GatherInfo(GatherType::GatherFilesOnLoadNotGathered, GatherFuncType::GatherFilesOnLoad, ErrorInfoInPWSTR(), (UINT32)lastErrorInfo.length()));
            return;
        }
        handlesInfo = (PSYSTEM_HANDLE_INFORMATION_EX)std::malloc(handlesInfoSize);
    }
    
    if (!NT_SUCCESS(status)) {
        SendToDT(
            currBuff,
            new GatherInfo(GatherType::GatherFilesOnLoadNotGathered, GatherFuncType::GatherFilesOnLoad));
        std::free(handlesInfo);
        return;
    }
    
    ULONG_PTR pid = GetCurrentProcessId();
    
    for(ULONG_PTR i = 0; i < handlesInfo->Length; i++) {
        PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX handleInfo = &handlesInfo->Handles[i];
        if(handleInfo->Pid == pid && handleInfo->TypeIndex == fileHandleTypeNumber) {
            GatherInfo *tmpInfo = FileHandleToInfoObject((HANDLE)handleInfo->Handle, GatherFuncType::GatherFilesOnLoad);
            if (tmpInfo != NULL) {
                SendToDT(currBuff, tmpInfo);
            }
        }
    }
    
    std::free(handlesInfo);
}

void SearchFileHandles(BuffObject * &currBuff) {
    if(isSupportInfoProc) {
        SearchFileHandlesModern(currBuff);
    }
    else {
        SearchFileHandlesLegacy(currBuff);
    }
}

void GetFeaturesSupport() {
    const ULONG win8BuildNumber = 9200,
        winBlueBuildNumber = 9600;
    _RtlGetNtVersionNumbers RtlGetNtVersionNumbers = 
                (_RtlGetNtVersionNumbers)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetNtVersionNumbers");
    if(RtlGetNtVersionNumbers == NULL) {
        return;
    }
    ULONG buildNum;
    RtlGetNtVersionNumbers(NULL, NULL, &buildNum);
    isSupportInfoProc = buildNum >= win8BuildNumber;
    isSupportTypeIndexInHandleInfoProc = buildNum >= winBlueBuildNumber;
}
