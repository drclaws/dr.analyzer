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

        private readonly System.Windows.Forms.Control logOutputWidget;

        private readonly Info.InfoBuilder infoBuilder;

        private readonly Queue<byte[]> messagesQueue;
        private MemoryMappedFile ipcMemory;
        private Mutex ipcMutex;
        private Semaphore ipcSemaphore;
        private Semaphore ipcWaiterSemaphore;

        private Thread recieverThread;

        public MessageConverter(System.Windows.Forms.Control logOutputWidget)
        {
            this.resourcesDictionary = new Dictionary<string, Info.GatheredResource>();
            this.logOutputWidget = logOutputWidget;
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

        public void ConnectByExe(string path)
        {
            throw new NotImplementedException(); 
        }

        private void RecieverThreadFunc()
        {
            while (true)
            {
                MemoryMappedViewStream viewStream;
                byte[] sizeBuff = new byte[4], message;
                bool exit = false;
                List<Info.IGatheredInfo> gatheredInfo;
                Console.WriteLine("START");
                this.ipcSemaphore.WaitOne();
                this.ipcMutex.WaitOne();

                viewStream = this.ipcMemory.CreateViewStream();
                viewStream.Read(sizeBuff, 0, 4);
                int size = BitConverter.ToInt32(sizeBuff, 0);

                message = new byte[size];
                viewStream.Read(message, 0, size);

                this.ipcMutex.ReleaseMutex();

                gatheredInfo = this.infoBuilder.ToInfoType(message);


                foreach (Info.IGatheredInfo info in gatheredInfo)
                {
                    if(info.Type == GatherType.GatherDeactivated)
                    {
                        exit = true;
                    }
                    Console.WriteLine("{0}", info.AsTextMessage());
                    //this.logOutputWidget.Text += info.AsTextMessage() + '\n';
                }


                if (exit)
                {
                    break;
                }
            }
        }
    }
}
