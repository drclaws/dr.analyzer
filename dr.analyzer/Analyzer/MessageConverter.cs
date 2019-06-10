using System;
using System.Collections.Generic;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

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


        public MessageConverter(System.Windows.Forms.Control logOutputWidget)
        {
            this.resourcesDictionary = new Dictionary<string, Info.GatheredResource>();
            this.logOutputWidget = logOutputWidget;
            this.infoBuilder = new Info.InfoBuilder();
            this.messagesQueue = new Queue<byte[]>();
        }

        public void ConnectByPid(Int32 pid)
        {
            this.ipcMemory = MemoryMappedFile.CreateNew(String.Format(@"Global\dr_analyzer_buffer_{0}", pid), 76012);
            this.ipcMutex = new Mutex(false, String.Format(@"Global\dr_analyzer_mutex_{0}", pid));
            this.ipcSemaphore = new Semaphore(0, 1, String.Format(@"Global\dr_analyzer_semaphore_{0}", pid));

            // TODO start reciever and collector threads

            Injector.InjectByPid(pid);
        }

        public void ConnectByExe(string path)
        {
            throw new NotImplementedException(); 
        }

        public void CreateRecieverThread()
        {

        }
    }
}
