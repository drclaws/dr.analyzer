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
using System.Text.RegularExpressions;
using DrAnalyzer.Analyzer;

namespace DrAnalyzer
{
    public partial class MainForm : Form
    {
        private List<Analyzer.Info.GatheredInfo> addedList;
        private Dictionary<string, Analyzer.Info.GatheredInfo> modulesList;
        private Dictionary<string, Analyzer.Info.GatheredInfo> filesList;
        private readonly List<string> fileDirsList;
        private Mutex syncObj;

        public Analyzer.MessageConverter converter;
        private Tree.TreeContainer treeContainer = new Tree.TreeContainer();

        private System.Windows.Forms.Timer timer;

        private bool started = false;

        public MainForm()
        {
            InitializeComponent();
            
            this.Icon = (System.Drawing.Icon)Icon.ExtractAssociatedIcon(System.Reflection.Assembly.GetExecutingAssembly().Location);

            ImageList iconsList = new ImageList();
            iconsList.Images.Add(Icon.ExtractAssociatedIcon(@"Icons\folder.ico"));
            iconsList.Images.Add(Icon.ExtractAssociatedIcon(@"Icons\module.ico"));
            iconsList.Images.Add(Icon.ExtractAssociatedIcon(@"Icons\file.ico"));

            this.treeView1.ImageList = iconsList;
            this.treeView1.ImageIndex = 0;
            this.treeView1.SelectedImageIndex = 0;

            this.modulesList = new Dictionary<string, Analyzer.Info.GatheredInfo>();
            this.filesList = new Dictionary<string, Analyzer.Info.GatheredInfo>();
            this.addedList = new List<Analyzer.Info.GatheredInfo>();
            this.fileDirsList = new List<string>();
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

                this.treeContainer.Clear();
                this.treeView1.Nodes.Clear();

                this.textBox1.Text = "";
                this.filesList.Clear();
                this.modulesList.Clear();
                this.fileDirsList.Clear();
                this.startButton.Text = "Stop";
                this.pidTextBox.ReadOnly = true;
                this.started = true;

                this.timer.Start();

                this.saveButton.Enabled = true;
            }
        }
        
        public void AddInfo(List<Analyzer.Info.GatheredInfo> info)
        {
            this.syncObj.WaitOne();
            this.addedList.AddRange(info);
            this.syncObj.ReleaseMutex();
        }

        private void TimerFunc(Object myObject, EventArgs myEventArgs)
        {
            this.syncObj.WaitOne();

            if (this.addedList.Count == 0)
            {
                this.syncObj.ReleaseMutex();
                return;
            }

            Analyzer.Info.GatheredInfo[] infos = this.addedList.ToArray();

            this.addedList.Clear();

            this.syncObj.ReleaseMutex();

            string textlog = "";
            bool isAdded = false;
            foreach(Analyzer.Info.GatheredInfo info in infos)
            {
                textlog += info.AsTextMessage() + "\r\n";

                Type objType = info.GetType();
                
                if (objType == typeof(Analyzer.Info.GatheredResource) ||
                    objType.IsSubclassOf(typeof(Analyzer.Info.GatheredResource)))
                {
                    string name;
                    switch(info.Type)
                    {
                        case Analyzer.GatherType.GatherFile:
                            name = info.Description;
                                if (name.StartsWith(@"\\?\"))
                                {
                                    name = name.Remove(0, 4);
                                }
                                name = System.IO.Path.GetFullPath(name).ToLower();
                                if (!this.filesList.ContainsKey(name))
                                {
                                    bool isFile;
                                    try
                                    {
                                        FileAttributes attrs = File.GetAttributes(name);
                                        isFile = !(attrs.HasFlag(FileAttributes.Directory) || attrs.HasFlag(FileAttributes.Temporary)); 
                                    } catch (Exception) { continue; }

                                    if (isFile)
                                    {
                                        this.filesList.Add(name, info);
                                        if (!this.modulesList.ContainsKey(name))
                                        {
                                            TreeNode newNode;
                                            if ((newNode = this.treeContainer.AddPath(name, false)) != null)
                                            {
                                                this.treeView1.Nodes.Add(newNode);
                                            }
                                            isAdded = true;
                                        }
                                    }
                                }
                            break;
                        
                        case Analyzer.GatherType.GatherLibrary:
                            name = System.IO.Path.GetFullPath(info.Description).ToLower();
                            if (!this.modulesList.ContainsKey(name))
                            {
                                this.modulesList.Add(name, info);
                                if (this.filesList.ContainsKey(name))
                                {
                                    this.treeContainer.MakeModuleByPath(name);
                                }
                                else
                                {
                                    TreeNode newNode;
                                    if ((newNode = this.treeContainer.AddPath(name, true)) != null)
                                    {
                                        this.treeView1.Nodes.Add(newNode);
                                    }
                                    isAdded = true;
                                }
                            }
                            break;
                    }
                }
                else if (objType == typeof(Analyzer.Info.NotGatheredError) ||
                         info.Type.HasFlag(GatherType.GatherStopped))
                {
                    this.timer.Stop();
                    this.startButton.Text = "Start";
                    this.pidTextBox.ReadOnly = false;
                    this.started = false;
                    this.startButton.Enabled = true;
                    break;
                }
            }
            this.textBox1.Text += textlog;
            if (isAdded)
            {
                this.treeView1.Sort();
            }
        }

        private void TextBox1_TextChanged(object sender, EventArgs e)
        {
            textBox1.SelectionStart = textBox1.Text.Length;
            textBox1.ScrollToCaret();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            this.timer.Stop();
            if (this.converter.Active)
            {
                this.converter.AbortGathering();
            }
        }

        private void saveButton_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog
            {
                Filter = "Text file|*.txt|All Files|*",
                Title = "Save an files list"
            };
            saveFileDialog1.ShowDialog();

            if (saveFileDialog1.FileName == "")
            {
                return;
            }
            
            System.IO.StreamWriter fs = new System.IO.StreamWriter(saveFileDialog1.OpenFile());

            bool anyModules = this.modulesList.Any(), anyFiles = this.filesList.Any();

            if (!(anyModules || anyFiles))
            {
                fs.WriteLine("File list empty");
            }
            else
            {
                if (anyModules)
                {
                    fs.WriteLine("Modules:");
                    foreach (Analyzer.Info.GatheredInfo module in this.modulesList.Values)
                    {
                        fs.WriteLine(module.Description);
                    }
                    fs.WriteLine();
                }

                if (anyFiles)
                {
                    fs.WriteLine("Files:");
                    foreach (Analyzer.Info.GatheredInfo file in this.filesList.Values)
                    {
                        fs.WriteLine(file.Description);
                    }
                }
            }
            fs.Close();
        }
    }
}
