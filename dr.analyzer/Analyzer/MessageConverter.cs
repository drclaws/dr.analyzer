using System;
using System.Collections.Generic;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Management;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Security.AccessControl;
using System.Security.Principal;

namespace DrAnalyzer.Analyzer
{
    public class MessageConverter
    {
        private readonly Dictionary<string, Info.GatheredResource> resourcesDictionary;

        private readonly MainForm mainFormClass;

        private readonly Info.InfoBuilder infoBuilder;

        private readonly Queue<byte[]> messagesQueue;
        private MemoryMappedFile ipcMemory;
        private Mutex ipcMutex;
        private Semaphore ipcSemaphore;
        private Semaphore ipcWaiterSemaphore;

        private Thread recieverThread;

        public MessageConverter(MainForm formClass)
        {
            this.resourcesDictionary = new Dictionary<string, Info.GatheredResource>();
            this.mainFormClass = formClass;
            this.infoBuilder = new Info.InfoBuilder();
            this.messagesQueue = new Queue<byte[]>();
        }

        public void ConnectByPid(Int32 pid)
        {        
            var securityMemory = new MemoryMappedFileSecurity();
            var securityMutex = new MutexSecurity();
            var securitySemaphore = new SemaphoreSecurity();
            var securityWaiter = new SemaphoreSecurity();

            securityMemory.AddAccessRule(new AccessRule<MemoryMappedFileRights>(new SecurityIdentifier(WellKnownSidType.WorldSid, null), MemoryMappedFileRights.FullControl, AccessControlType.Allow));
            securityMutex.AddAccessRule(new MutexAccessRule(new SecurityIdentifier(WellKnownSidType.WorldSid, null), MutexRights.FullControl, AccessControlType.Allow));
            securitySemaphore.AddAccessRule(new SemaphoreAccessRule(new SecurityIdentifier(WellKnownSidType.WorldSid, null), SemaphoreRights.FullControl, AccessControlType.Allow));
            securityWaiter.AddAccessRule(new SemaphoreAccessRule(new SecurityIdentifier(WellKnownSidType.WorldSid, null), SemaphoreRights.FullControl, AccessControlType.Allow));

            this.ipcMemory = MemoryMappedFile.CreateNew(String.Format("Global\\dr_analyzer_buffer_{0}", pid), 76012);
            this.ipcMemory.SetAccessControl(securityMemory);
            this.ipcMutex = new Mutex(false, String.Format("Global\\dr_analyzer_mutex_{0}", pid), out bool createdMutex, securityMutex);
            this.ipcSemaphore = new Semaphore(0, 1, String.Format("Global\\dr_analyzer_semaphore_{0}", pid), out bool createdSemaphore, securitySemaphore);
            this.ipcWaiterSemaphore = new Semaphore(0, 1, String.Format("Global\\dr_analyzer_waiter_semaphore_{0}", pid), out bool createdWaiter, securityWaiter);

            if (!(createdMutex && createdSemaphore && createdWaiter))
            {
                throw new Exception("One of sync object is already created");
            }

            this.recieverThread = new Thread(this.RecieverThreadFunc);

            this.recieverThread.Start();

            Injector.InjectByPid(pid);
        }

        public void StopGathering()
        {
            this.ipcWaiterSemaphore.Release();
        }

        private void RecieverThreadFunc()
        {
            byte[] zeroSizeValue = new byte[] { 0, 0, 0, 0 };
            while (true)
            {
                MemoryMappedViewStream viewStream;
                byte[] sizeBuff = new byte[4], message;
                List<Info.IGatheredInfo> gatheredInfo;

                this.ipcSemaphore.WaitOne(10000);
                this.ipcMutex.WaitOne(1000);
                
                viewStream = this.ipcMemory.CreateViewStream();
                viewStream.Read(sizeBuff, 0, 4);
                int size = BitConverter.ToInt32(sizeBuff, 0);
                
                if (size == 0)
                {
                    gatheredInfo = new List<Info.IGatheredInfo>
                    {
                        new Info.NotGatheredError()
                    };
                    this.mainFormClass.AddInfo(gatheredInfo);
                    break;
                }

                message = new byte[size];
                viewStream.Read(message, 0, size);
                viewStream.Position = 0;
                viewStream.Write(zeroSizeValue, 0, 4);
                viewStream.Flush();
                this.ipcMutex.ReleaseMutex();

                gatheredInfo = this.infoBuilder.ToInfoType(message);

                this.mainFormClass.AddInfo(gatheredInfo);

                if (gatheredInfo.Exists(info => info.Type == GatherType.GatherDeactivated))
                {
                    break;
                }
            }
        }
    }
}
