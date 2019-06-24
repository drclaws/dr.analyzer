using System;
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
            this.checkBox1_CheckStateChanged(null, null);
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
                this.pidTextBox.ReadOnly = true;
                this.textBox2.Enabled = false;
                this.label2.Enabled = false;
                this.button1.Enabled = false;
                this.started = true;
                this.label5.Enabled = false;
                this.checkBox1.Enabled = false;

                this.saveButton.Enabled = true;
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
                        {
                            string name = info.Name.ToLower();
                            if (name.StartsWith(@"\\?\"))
                            {
                                name = name.Remove(0, 4);
                            }
                            if (!this.filesList.ContainsKey(name))
                            {
                                this.filesList.Add(name, info);
                                this.listBox2.Items.Add(name);
                            }
                        }
                        break;
                    case Analyzer.GatherType.GatherLibrary:
                        {
                            string name = System.IO.Path.GetFullPath(info.Name.ToLower());
                            if (!this.modulesList.ContainsKey(name))
                            {
                                this.modulesList.Add(name, info);
                                this.listBox1.Items.Add(name);
                            }
                        }
                        break;
                    case Analyzer.GatherType.GatherDeactivated:
                        this.timer.Stop();
                        this.startButton.Text = "Start";
                        this.pidTextBox.ReadOnly = false;
                        this.label5.Enabled = true;
                        this.checkBox1.Enabled = true;
                        this.checkBox1_CheckStateChanged(null, null);
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

        private void checkBox1_CheckStateChanged(object sender, EventArgs e)
        {
            if (this.checkBox1.Checked == true)
            {
                this.label2.Enabled = true;
                this.textBox2.Enabled = true;
                this.button1.Enabled = true;
            }
            else
            {
                this.label2.Enabled = false;
                this.textBox2.Enabled = false;
                this.button1.Enabled = false;
            }
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {

        }
    }
}
