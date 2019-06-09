namespace DrAnalyzer
{
    partial class MainForm
    {
        /// <summary>
        /// Обязательная переменная конструктора.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Освободить все используемые ресурсы.
        /// </summary>
        /// <param name="disposing">истинно, если управляемый ресурс должен быть удален; иначе ложно.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Код, автоматически созданный конструктором форм Windows

        /// <summary>
        /// Требуемый метод для поддержки конструктора — не изменяйте 
        /// содержимое этого метода с помощью редактора кода.
        /// </summary>
        private void InitializeComponent()
        {
            this.exeRadioButton = new System.Windows.Forms.RadioButton();
            this.pidRadioButton = new System.Windows.Forms.RadioButton();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exePathTextBox = new System.Windows.Forms.TextBox();
            this.exeBrowseButton = new System.Windows.Forms.Button();
            this.pidTextBox = new System.Windows.Forms.TextBox();
            this.startButton = new System.Windows.Forms.Button();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // exeRadioButton
            // 
            this.exeRadioButton.AutoSize = true;
            this.exeRadioButton.Checked = true;
            this.exeRadioButton.Location = new System.Drawing.Point(12, 32);
            this.exeRadioButton.Name = "exeRadioButton";
            this.exeRadioButton.Size = new System.Drawing.Size(89, 17);
            this.exeRadioButton.TabIndex = 0;
            this.exeRadioButton.TabStop = true;
            this.exeRadioButton.Text = "Start Program";
            this.exeRadioButton.UseVisualStyleBackColor = true;
            this.exeRadioButton.CheckedChanged += new System.EventHandler(this.exeRadioButton_CheckedChanged);
            // 
            // pidRadioButton
            // 
            this.pidRadioButton.AutoSize = true;
            this.pidRadioButton.Location = new System.Drawing.Point(12, 67);
            this.pidRadioButton.Name = "pidRadioButton";
            this.pidRadioButton.Size = new System.Drawing.Size(100, 17);
            this.pidRadioButton.TabIndex = 1;
            this.pidRadioButton.Text = "Connect by PID";
            this.pidRadioButton.UseVisualStyleBackColor = true;
            this.pidRadioButton.CheckedChanged += new System.EventHandler(this.pidRadioButton_CheckedChanged);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(736, 24);
            this.menuStrip1.TabIndex = 2;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // exePathTextBox
            // 
            this.exePathTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.exePathTextBox.Location = new System.Drawing.Point(121, 31);
            this.exePathTextBox.Name = "exePathTextBox";
            this.exePathTextBox.Size = new System.Drawing.Size(523, 20);
            this.exePathTextBox.TabIndex = 3;
            // 
            // exeBrowseButton
            // 
            this.exeBrowseButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.exeBrowseButton.Location = new System.Drawing.Point(650, 29);
            this.exeBrowseButton.Name = "exeBrowseButton";
            this.exeBrowseButton.Size = new System.Drawing.Size(75, 23);
            this.exeBrowseButton.TabIndex = 4;
            this.exeBrowseButton.Text = "Browse";
            this.exeBrowseButton.UseVisualStyleBackColor = true;
            this.exeBrowseButton.Click += new System.EventHandler(this.exeBrowseButton_Click);
            // 
            // pidTextBox
            // 
            this.pidTextBox.Location = new System.Drawing.Point(121, 66);
            this.pidTextBox.Name = "pidTextBox";
            this.pidTextBox.Size = new System.Drawing.Size(118, 20);
            this.pidTextBox.TabIndex = 5;
            // 
            // startButton
            // 
            this.startButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.startButton.Location = new System.Drawing.Point(650, 102);
            this.startButton.Name = "startButton";
            this.startButton.Size = new System.Drawing.Size(75, 23);
            this.startButton.TabIndex = 6;
            this.startButton.Text = "Start";
            this.startButton.UseVisualStyleBackColor = true;
            // 
            // textBox1
            // 
            this.textBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBox1.BackColor = System.Drawing.SystemColors.Window;
            this.textBox1.Location = new System.Drawing.Point(12, 131);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.ReadOnly = true;
            this.textBox1.Size = new System.Drawing.Size(712, 292);
            this.textBox1.TabIndex = 7;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(736, 435);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.startButton);
            this.Controls.Add(this.pidTextBox);
            this.Controls.Add(this.exeBrowseButton);
            this.Controls.Add(this.exePathTextBox);
            this.Controls.Add(this.pidRadioButton);
            this.Controls.Add(this.exeRadioButton);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.MinimumSize = new System.Drawing.Size(400, 333);
            this.Name = "MainForm";
            this.Text = "Form1";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.RadioButton exeRadioButton;
        private System.Windows.Forms.RadioButton pidRadioButton;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.TextBox exePathTextBox;
        private System.Windows.Forms.Button exeBrowseButton;
        private System.Windows.Forms.TextBox pidTextBox;
        private System.Windows.Forms.Button startButton;
        private System.Windows.Forms.TextBox textBox1;
    }
}

