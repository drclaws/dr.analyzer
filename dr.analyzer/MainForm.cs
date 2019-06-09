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
        public MainForm()
        {
            InitializeComponent();
            this.RadioChanged(true);
        }

        private void exeBrowseButton_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Multiselect = false;
                openFileDialog.Filter = "Executable files (*.exe)|*.exe|All files (*.*)|*.*";
                openFileDialog.FilterIndex = 1;
                openFileDialog.CheckFileExists = true;

                if (System.IO.File.Exists(this.exePathTextBox.Text))
                {
                    openFileDialog.FileName = Path.GetFileName(this.exePathTextBox.Text);
                    openFileDialog.InitialDirectory = Path.GetDirectoryName(this.exePathTextBox.Text);
                }
                else
                {
                    String dir;
                    try
                    {
                        dir = Path.GetDirectoryName(this.exePathTextBox.Text);
                        if (System.IO.Directory.Exists(dir))
                        {
                            openFileDialog.InitialDirectory = dir;
                        }
                        else
                        {
                            openFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
                        }
                    }
                    catch (Exception)
                    {
                        openFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
                    }
                }

                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {
                    this.exePathTextBox.Text = openFileDialog.FileName;
                }
            }
        }

        private void exeRadioButton_CheckedChanged(object sender, EventArgs e)
        {
            if (((RadioButton)sender).Checked)
            {
                this.RadioChanged(true);
            }
        }

        private void pidRadioButton_CheckedChanged(object sender, EventArgs e)
        {
            if (((RadioButton)sender).Checked)
            {
                this.RadioChanged(false);
            }
        }

        private void RadioChanged(bool isExe)
        {
            this.exePathTextBox.Enabled = isExe;
            this.exeBrowseButton.Enabled = isExe;
            this.pidTextBox.Enabled = !isExe;
        }
    }
}
