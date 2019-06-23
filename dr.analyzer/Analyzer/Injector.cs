using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace DrAnalyzer.Analyzer
{
    public static class Injector
    {
        const uint INFINITE = 0xFFFFFFFF;
        private static String dllPath = "";
        private static bool dllPathSet = false;
        private static String DllPath
        {
            get
            {
                if (!Injector.dllPathSet)
                {
#if DEBUG
                    Injector.dllPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + "\\..\\Release\\hook.dll";
                    Injector.dllPath = System.IO.Path.GetFullPath(dllPath);
#else
                    Injector.dllPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + "\\hook.dll";
#endif
                    Injector.dllPathSet = true;
                }
                return dllPath;
            }
        }

        //public static bool IsSet { get; private set; } = false;
        
        public static void InjectByPid(Int32 pid)
        {
            IntPtr openedProcess = ImportedFuncs.OpenProcess(ProcessAccessFlags.All, false, pid);
            IntPtr kernelModule = ImportedFuncs.GetModuleHandle("kernel32.dll");
            IntPtr loadLibratyAddr = ImportedFuncs.GetProcAddress(kernelModule, "LoadLibraryW");
            
            byte[] dllPathBytes = System.Text.Encoding.Unicode.GetBytes(Injector.DllPath);
            Int32 sizeOfPath = dllPathBytes.Length;
            IntPtr sizePtr = new IntPtr(sizeOfPath);
            UIntPtr uSizePtr = new UIntPtr((uint)sizeOfPath);

            IntPtr argLoadLibrary = ImportedFuncs.VirtualAllocEx(openedProcess, IntPtr.Zero, sizePtr, AllocationType.Reserve | AllocationType.Commit, MemoryProtection.ReadWrite);

            if (argLoadLibrary == IntPtr.Zero)
            {
                ImportedFuncs.CloseHandle(openedProcess);
                throw new Exception("Injection: Cannot allocate memory");
            }

            IntPtr writedBytesCount;

            Boolean writed = ImportedFuncs.WriteProcessMemory(openedProcess, argLoadLibrary, System.Text.Encoding.Unicode.GetBytes(Injector.DllPath), uSizePtr, out writedBytesCount);

            if (!writed)
            {
                ImportedFuncs.VirtualFree(openedProcess, argLoadLibrary, AllocationType.Release | AllocationType.Commit);
                ImportedFuncs.CloseHandle(openedProcess);
                throw new Exception("Injection: Cannot write in allocated memory");
            }

            IntPtr threadIdOut;
            IntPtr threadId = ImportedFuncs.CreateRemoteThread(openedProcess, IntPtr.Zero, 0, loadLibratyAddr, argLoadLibrary, 0, out threadIdOut);

            if (threadId == null)
            {
                ImportedFuncs.VirtualFree(openedProcess, argLoadLibrary, AllocationType.Release | AllocationType.Commit);
                ImportedFuncs.CloseHandle(openedProcess);
                throw new Exception("Injection: Cannot create remote thread");
            }

            ImportedFuncs.CloseHandle(threadId);
        }
    }
}
