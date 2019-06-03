using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using dr_analyzer.flags;


namespace dr_analyzer
{
    public static class Injector
    {
        private static String dllPath = "";
        private static bool dllPathSet = false;
        private static String DllPath
        {
            get
            {
                if (dllPathSet)
                {
#if DEBUG
                    dllPath = System.Reflection.Assembly.GetExecutingAssembly().Location + @"..\Release\hook.dll";
                    dllPath = System.IO.Path.GetFullPath(dllPath);
#else
                    dllPath = System.Reflection.Assembly.GetExecutingAssembly().Location + @"\hook.dll";
#endif
                    dllPathSet = true;
                }
                return dllPath;
            }
        }

        public static bool isSet { get; private set; } = false;

        public static void InjectByPid(Int32 pid)
        {
            if (isSet)
            {
                throw new Exception("It's already injected");
            }

            IntPtr openedProcess = Exporter.OpenProcess(ProcessAccessFlags.All, false, pid);
            IntPtr kernelModule = Exporter.GetModuleHandle("kernel32.dll");
            IntPtr loadLibratyAddr = Exporter.GetProcAddress(kernelModule, "LoadLibraryA");

            Int32 len = DllPath.Length;
            IntPtr lenPtr = new IntPtr(len);
            UIntPtr uLenPtr = new UIntPtr((uint)len);

            IntPtr argLoadLibrary = Exporter.VirtualAllocEx(openedProcess, IntPtr.Zero, lenPtr, AllocationType.Reserve | AllocationType.Commit, MemoryProtection.ReadWrite);

            IntPtr writedBytesCount;

            Boolean writed = Exporter.WriteProcessMemory(openedProcess, argLoadLibrary, System.Text.Encoding.ASCII.GetBytes(DllPath), uLenPtr, out writedBytesCount);

            IntPtr threadIdOut;
            IntPtr threadId = Exporter.CreateRemoteThread(openedProcess, IntPtr.Zero, 0, loadLibratyAddr, argLoadLibrary, 0, out threadIdOut);

            Exporter.CloseHandle(threadId);

            isSet = true;
        }

        public static void InjectByPath(String exePath)
        {

        }

        public static void Deinject()
        {
            if (!isSet)
            {
                throw new Exception("It has not been injected");
            }
        }

    }
}
