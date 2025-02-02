﻿using System;
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

        private MemoryMappedFile ipcMemory = null;
        private Mutex ipcMutex = null;
        private Semaphore ipcSentSemaphore = null;
        private Semaphore ipcReceivedSemaphore = null;
        private Semaphore ipcWaiterSemaphore = null;

        private Thread receiverThread;
        private Thread queueThread;
        private readonly Queue<byte[]> messagesQueue;
        private Semaphore queueSem;
        private Mutex queueMutex;

        private bool isExit;

        public bool Active { get; private set; } = false;

        public MessageConverter(MainForm formClass)
        {
            this.resourcesDictionary = new Dictionary<string, Info.GatheredResource>();
            this.mainFormClass = formClass;
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

            this.ipcMemory = MemoryMappedFile.CreateNew($"Global\\dr_analyzer_buffer_{pid}", 76012);
            this.ipcMemory.SetAccessControl(securityMemory);
            this.ipcMutex = new Mutex(false, $"Global\\dr_analyzer_mutex_{pid}", out bool createdMutex, securityMutex);
            this.ipcSentSemaphore = new Semaphore(0, 1, $"Global\\dr_analyzer_sent_semaphore_{pid}", out bool createdSentSemaphore, securitySemaphore);
            this.ipcReceivedSemaphore = new Semaphore(0, 1, $"Global\\dr_analyzer_received_semaphore_{pid}", out bool createdReceivedSemaphore, securitySemaphore);
            this.ipcWaiterSemaphore = new Semaphore(0, 1, $"Global\\dr_analyzer_waiter_semaphore_{pid}", out bool createdWaiter, securityWaiter);

            if (!(createdMutex && createdSentSemaphore && createdReceivedSemaphore && createdWaiter))
            {
                this.FreeSharedObjects();
                throw new Exception("One of sync object is already created");
            }

            this.queueSem = new Semaphore(0, 1);
            this.queueMutex = new Mutex(false);
            this.isExit = false;

            this.receiverThread = new Thread(this.ReceiverThreadFunc);
            this.queueThread = new Thread(this.QueueThreadFunc);

            this.queueThread.Start();
            this.receiverThread.Start();

            Injector.InjectByPid(pid);

            this.Active = true;
        }

        public void StopGathering()
        {
            this.ipcWaiterSemaphore.Release();
        }

        private void FreeSharedObjects()
        {
            if (this.ipcWaiterSemaphore != null)
            {
                this.ipcWaiterSemaphore.SafeWaitHandle.Close();
                this.ipcWaiterSemaphore = null;
            }
            if (this.ipcReceivedSemaphore != null) {
                this.ipcReceivedSemaphore.SafeWaitHandle.Close();
                this.ipcReceivedSemaphore = null;
            }
            if (this.ipcSentSemaphore != null)
            {
                this.ipcSentSemaphore.SafeWaitHandle.Close();
                this.ipcSentSemaphore = null;
            }
            if (this.ipcMutex != null)
            {
                this.ipcMutex.SafeWaitHandle.Close();
                this.ipcMutex = null;
            }
            if (this.ipcMemory != null)
            {
                this.ipcMemory.SafeMemoryMappedFileHandle.Close();
                this.ipcMemory = null;
            }
        }

        private void QueueThreadFunc()
        {
            while (true)
            {
                bool disconnectReceived = false;

                this.queueSem.WaitOne();

                List<Info.GatheredInfo> gatheredInfo;

                if (isExit)
                {
                    gatheredInfo = new List<Info.GatheredInfo> { new Info.NotGatheredError() };
                    this.mainFormClass.AddInfo(gatheredInfo);
                    this.Active = false;
                    this.FreeSharedObjects();
                    break;
                }

                while (true)
                {
                    byte[] message;

                    this.queueMutex.WaitOne();
                    try
                    {
                        message = this.messagesQueue.Dequeue();
                    }
                    catch (InvalidOperationException)
                    {
                        this.queueMutex.ReleaseMutex();
                        break;
                    }
                    this.queueMutex.ReleaseMutex();

                    gatheredInfo = Info.InfoBuilder.ToInfoType(message);
                    this.mainFormClass.AddInfo(gatheredInfo);

                    if (gatheredInfo.Exists(info => info.Type == GatherType.GatherStopped))
                    {
                        disconnectReceived = true;
                        break;
                    }
                }

                if (disconnectReceived)
                {
                    this.isExit = true;
                    this.ipcSentSemaphore.Release();
                    this.receiverThread.Join();
                    this.Active = false;
                    this.FreeSharedObjects();
                    break;
                }
            }
        }

        private void ReceiverThreadFunc()
        {
            byte[] zeroSizeValue = new byte[] { 0, 0, 0, 0 }, sizeBuff = new byte[4];
            MemoryMappedViewStream viewStream = this.ipcMemory.CreateViewStream();
            System.Diagnostics.Stopwatch watch = new System.Diagnostics.Stopwatch();

            while (true)
            {
                watch.Start();
                this.ipcSentSemaphore.WaitOne(10000);
                watch.Stop();
                if (this.isExit)
                {
                    break;
                }
                this.ipcMutex.WaitOne();

                viewStream.Position = 0;
                viewStream.Read(sizeBuff, 0, 4);
                int size = BitConverter.ToInt32(sizeBuff, 0);
                
                if (size == 0)
                {
                    this.ipcMutex.ReleaseMutex();
                    if (watch.ElapsedMilliseconds < 10000)
                    {
                        this.ipcReceivedSemaphore.Release();
                        continue;
                    }
                    this.isExit = true;
                    this.queueSem.Release();
                    this.queueThread.Join();
                    break;
                }

                byte[] message = new byte[size];
                viewStream.Read(message, 0, size);
                viewStream.Position = 0;
                viewStream.Write(zeroSizeValue, 0, 4);
                viewStream.Flush();
                this.ipcMutex.ReleaseMutex();
                this.ipcReceivedSemaphore.Release();

                this.queueMutex.WaitOne();
                this.messagesQueue.Enqueue(message);
                this.queueMutex.ReleaseMutex();
                try
                {
                    this.queueSem.Release();
                }
                catch(SemaphoreFullException) { }
            }
        }

        public void AbortGathering()
        {
            this.StopGathering();
            this.queueThread.Join();
        }
    }
}
