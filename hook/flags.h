#pragma once
#include "stdafx.h"

typedef UINT16 gather_flag_t;


enum GatherFuncType {
    // Hook's sender locations
    GatherConnection = 0x0000,
    GatherFilesOnLoad,
    GatherWaiter,
    GatherBufferSender,
    
    // Detoured file opening locations
    GatherCreateFile2 = 0x0010,
    GatherCreateFileA,
    GatherCreateFileW,
    GatherOpenFileById,

    // Detoured module loading locations
    GatherLoadLibraryA = 0x0020,
    GatherLoadLibraryW,    
    GatherLoadLibraryExA,
    GatherLoadLibraryExW,
        
    // GUI-app's receiver locations
    GatherReceiverThread = 0x0030
};


enum GatherType {
    GatherNone =                    (gather_flag_t)0x0000,
    
    // FIXED BITS (all abstract)
    GatherError =                   (gather_flag_t)0x8000,
    GatherWarning =                 (gather_flag_t)0x4000,
    
    GatherHasValue =                (gather_flag_t)0x2000, // defines that message will have additional some data
    
    GatherStatus =                  (gather_flag_t)0x1000, // connection status group   
    GatherResource =                (gather_flag_t)0x0800, // collected data group
        
    // FIXED BITS PAIRS / DATA
    GatherFile =                    (gather_flag_t)0x0100 | GatherResource,
    GatherLibrary =                 (gather_flag_t)0x0200 | GatherResource,
    GatherReservedResource =        (gather_flag_t)0x0400 | GatherResource,
    
    // DATA
    // Statuses
    GatherStarted =                 (gather_flag_t)0x0000 | GatherStatus,
    GatherStopped =                 (gather_flag_t)0x0001 | GatherStatus,
    GatherStillUp =                 (gather_flag_t)0x0002 | GatherStatus,
    
    // WARNINGS
    // Collected data
    GatherPathToBig =               (gather_flag_t)0x0000 | GatherWarning | GatherResource, // abstract
    GatherFilePathToBig =           GatherPathToBig | GatherFile,
    GatherModulePathToBig =         GatherPathToBig | GatherLibrary,
    
    GatherCannotGetPath =           (gather_flag_t)0x0001 | GatherWarning | GatherResource, // abstract
    GatherFileCannotGetPath =       GatherCannotGetPath | GatherFile,
    GatherLibraryCannotGetPath =    GatherCannotGetPath | GatherLibrary,
    
    // ERRORS
    // Statuses
    GatherDetourError =             (gather_flag_t)0x0000 | GatherError | GatherStatus,
    GatherUndetourError =           (gather_flag_t)0x0001 | GatherError | GatherStatus,
    GatherWaiterError =             (gather_flag_t)0x0002 | GatherError | GatherStatus,
    
    // Collected data
    GatherInfoOnLoadNotGathered =   (gather_flag_t)0x0000 | GatherError | GatherResource, // abstract
    GatherFilesOnLoadNotGathered =  GatherInfoOnLoadNotGathered | GatherFile,
    GatherLibrariesOnLoadNotGathered = GatherInfoOnLoadNotGathered | GatherLibrary
};