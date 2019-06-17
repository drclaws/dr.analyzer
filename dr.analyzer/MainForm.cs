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

namespace DrAnalyzer
{
    public partial class MainForm : Form
    {
        Analyzer.MessageConverter converter;
        public MainForm()
        {
            InitializeComponent();
            converter = new Analyzer.MessageConverter(this.textBox1);
        }

        private void StartButton_Click(object sender, EventArgs e)
        {
            converter.ConnectByPid(Convert.ToInt32(this.pidTextBox.Text));
        }
    }
}
