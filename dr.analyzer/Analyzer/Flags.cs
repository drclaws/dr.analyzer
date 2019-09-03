using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel;
using System.Reflection;

namespace DrAnalyzer.Analyzer
{
    public static class EnumExtensionMethods
    {
        public static string GetDescription(this Enum value)
        {
            Type type = value.GetType();
            string name = Enum.GetName(type, value);
            if (name != null)
            {
                FieldInfo field = type.GetField(name);
                if (field != null)
                {
                    if (Attribute.GetCustomAttribute(field,
                        typeof(DescriptionAttribute)) is DescriptionAttribute attr)
                    {
                        return attr.Description;
                    }
                }
            }
            return null;
        }
    }

    [Flags]
    public enum ProcessAccessFlags : uint
    {
        All =                       0x001F0FFF,
        Terminate =                 0x00000001,
        CreateThread =              0x00000002,
        VirtualMemoryOperation =    0x00000008,
        VirtualMemoryRead =         0x00000010,
        VirtualMemoryWrite =        0x00000020,
        DuplicateHandle =           0x00000040,
        CreateProcess =             0x000000080,
        SetQuota =                  0x00000100,
        SetInformation =            0x00000200,
        QueryInformation =          0x00000400,
        QueryLimitedInformation =   0x00001000,
        Synchronize =               0x00100000
    }

    [Flags]
    public enum AllocationType
    {
        Commit =        0x1000,
        Reserve =       0x2000,
        Decommit =      0x4000,
        Release =       0x8000,
        Reset =         0x80000,
        Physical =      0x400000,
        TopDown =       0x100000,
        WriteWatch =    0x200000,
        LargePages =    0x20000000
    }

    [Flags]
    public enum MemoryProtection
    {
        Execute =                   0x10,
        ExecuteRead =               0x20,
        ExecuteReadWrite =          0x40,
        ExecuteWriteCopy =          0x80,
        NoAccess =                  0x01,
        ReadOnly =                  0x02,
        ReadWrite =                 0x04,
        WriteCopy =                 0x08,
        GuardModifierflag =         0x100,
        NoCacheModifierflag =       0x200,
        WriteCombineModifierflag =  0x400
    }
    
    public enum GatherFuncType : UInt16
    {
        // Hook's sender locations
        [Description("Connection stage")]
        GatherConnection = 0x0000,
        [Description("FilesOnLoad")]
        GatherFilesOnLoad,
        [Description("Waiter thread")]
        GatherWaiter,
        [Description("Sender thread")]
        GatherBufferSender,
                                    
        // Detoured file opening locations
        [Description("CreateFile2")]
        GatherCreateFile2 = 0x0010,
        [Description("CreateFileA")]
        GatherCreateFileA,
        [Description("CreateFileW")]
        GatherCreateFileW,
        [Description("CreateFileById")]
        GatherOpenFileById,
        
        // Detoured module loading locations
        [Description("LoadLibraryA")]
        GatherLoadLibraryA = 0x0020,
        [Description("LoadLibraryW")]
        GatherLoadLibraryW,
        [Description("LoadLibraryExA")]
        GatherLoadLibraryExA,
        [Description("LoadLibraryExW")]
        GatherLoadLibraryExW,
        
        // GUI-app's receiver locations
        [Description("RecieverThreadFunc")]
        GatherReceiverThread = 0x0030
    }

    [Flags]    
    public enum GatherType : UInt16
    {
        GatherNone =                    0x0000,
    
        // FIXED BITS (all abstract)
        GatherError =                   0x8000,
        GatherWarning =                 0x4000,
        
        GatherHasValue =                0x2000, // defines that message will have additional some data
        
        GatherStatus =                  0x1000, // connection status group   
        GatherResource =                0x0800, // collected data group
        
        // FIXED BITS PAIRS / DATA
        [Description("File")]
        GatherFile =                    0x0100 | GatherResource,
        [Description("Library")]
        GatherLibrary =                 0x0200 | GatherResource,
        GatherReservedResource =        0x0400 | GatherResource,
        
        // DATA
        // Statuses
        [Description("Gathering has been started")]
        GatherStarted =                 0x0000 | GatherStatus,
        [Description("Gathering has been stopped")]
        GatherStopped =                 0x0001 | GatherStatus,
        [Description("Gathering is up")]
        GatherStillUp =                 0x0002 | GatherStatus,
    
        // WARNINGS
        // Collected data
        GatherPathToBig =               0x0000 | GatherWarning | GatherResource, // abstract
        [Description("Path to gathered file is to big")]
        GatherFilePathToBig =           GatherPathToBig | GatherFile,
        [Description("Path to gathered module is to big")]
        GatherModulePathToBig =         GatherPathToBig | GatherLibrary,
    
        GatherCannotGetPath =           0x0001 | GatherWarning | GatherResource, // abstract
        [Description("Cannot get filepath of gathered file handle")]
        GatherFileCannotGetPath =       GatherCannotGetPath | GatherFile,
        [Description("Cannot get filepath of gathered module handle")]
        GatherLibraryCannotGetPath =    GatherCannotGetPath | GatherLibrary,
        
        // ERRORS
        // Statuses
        [Description("Detouring analyzed calls error")]
        GatherDetourError =             0x0000 | GatherError | GatherStatus,
        [Description("Undetouring analyzed calls error")]
        GatherUndetourError =           0x0001 | GatherError | GatherStatus,
        [Description("Waiter thread error")]
        GatherWaiterError =             0x0002 | GatherError | GatherStatus,
        
        // Collected data
        GatherInfoOnLoadNotGathered =   0x0000 | GatherError | GatherResource, // abstract
        [Description("Error on getting info about opened file on analyze process start")]
        GatherFilesOnLoadNotGathered =  GatherInfoOnLoadNotGathered | GatherFile,
        [Description("Error on getting info about used modules on analyze process start")]
        GatherLibrariesOnLoadNotGathered = GatherInfoOnLoadNotGathered | GatherLibrary
    }
}
