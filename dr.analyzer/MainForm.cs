﻿using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Threading;

namespace DrAnalyzer
{
    public partial class MainForm : Form
    {
        private List<Analyzer.Info.IGatheredInfo> addedList;
        private Dictionary<string, Analyzer.Info.IGatheredInfo> modulesList;
        private Dictionary<string, Analyzer.Info.IGatheredInfo> filesList;
        private Mutex syncObj;

        public Analyzer.MessageConverter converter;

        private System.Windows.Forms.Timer timer;

        private bool started = false;


        public MainForm()
        {
            InitializeComponent();
            this.modulesList = new Dictionary<string, Analyzer.Info.IGatheredInfo>();
            this.filesList = new Dictionary<string, Analyzer.Info.IGatheredInfo>();
            this.addedList = new List<Analyzer.Info.IGatheredInfo>();
            this.syncObj = new Mutex();
            this.timer = new System.Windows.Forms.Timer { Interval = 300 };
            this.timer.Tick += new EventHandler(this.TimerFunc);

            this.converter = new Analyzer.MessageConverter(this);
        }

        private void StartButton_Click(object sender, EventArgs e)
        {
            if (started)
            {
                this.converter.StopGathering();
                this.startButton.Enabled = false;
            }
            else
            {
                converter.ConnectByPid(Convert.ToInt32(this.pidTextBox.Text));
                this.textBox1.Text = "";
                this.listBox1.Items.Clear();
                this.listBox2.Items.Clear();
                this.filesList.Clear();
                this.modulesList.Clear();
                this.timer.Start();
                this.startButton.Text = "Stop";
                this.pidTextBox.Enabled = false;
                this.textBox2.Enabled = false;
                this.button1.Enabled = false;
                this.started = true;
            }
        }

        public void AddInfo(List<Analyzer.Info.IGatheredInfo> info)
        {
            this.syncObj.WaitOne();
            this.addedList.AddRange(info);
            this.syncObj.ReleaseMutex();
        }

        public void TimerFunc(Object myObject, EventArgs myEventArgs)
        {
            this.syncObj.WaitOne();

            if (this.addedList.Count == 0)
            {
                this.syncObj.ReleaseMutex();
                return;
            }

            Analyzer.Info.IGatheredInfo[] infos = this.addedList.ToArray();

            this.addedList.Clear();

            this.syncObj.ReleaseMutex();

            string textlog = "";
            foreach(Analyzer.Info.IGatheredInfo info in infos)
            {
                textlog += info.AsTextMessage() + "\r\n";
                if (info is Analyzer.Info.GatheredWarning)
                {
                    continue;
                }
                switch(info.Type)
                {
                    case Analyzer.GatherType.GatherFile:
                        if (!this.filesList.ContainsKey(info.Name))
                        {
                            this.filesList.Add(info.Name, info);
                            this.listBox2.Items.Add(info.Name);
                        }
                        break;
                    case Analyzer.GatherType.GatherLibrary:
                        if (!this.modulesList.ContainsKey(info.Name))
                        {
                            this.modulesList.Add(info.Name, info);
                            this.listBox1.Items.Add(info.Name);
                        }
                        break;
                    case Analyzer.GatherType.GatherDeactivated:
                        this.timer.Stop();
                        this.startButton.Text = "Start";
                        this.pidTextBox.Enabled = true;
                        this.textBox2.Enabled = true;
                        this.button1.Enabled = true;
                        this.started = false;
                        this.startButton.Enabled = true;
                        break;
                    default:
                        break;
                }
            }
            this.textBox1.Text += textlog;
        }

        private void TextBox1_TextChanged(object sender, EventArgs e)
        {
            textBox1.SelectionStart = textBox1.Text.Length;
            textBox1.ScrollToCaret();
        }
    }
}
