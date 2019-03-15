namespace RGALayerLauncher
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.labelExe = new System.Windows.Forms.Label();
            this.labelOutputDir = new System.Windows.Forms.Label();
            this.textBoxExe = new System.Windows.Forms.TextBox();
            this.textBoxOutputDir = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.buttonOutputDir = new System.Windows.Forms.Button();
            this.buttonLaunch = new System.Windows.Forms.Button();
            this.button3 = new System.Windows.Forms.Button();
            this.buttonWorkingDir = new System.Windows.Forms.Button();
            this.textBoxWorkingDir = new System.Windows.Forms.TextBox();
            this.labelWorkingDir = new System.Windows.Forms.Label();
            this.textBoxPipelineNames = new System.Windows.Forms.TextBox();
            this.labelPipelineNames = new System.Windows.Forms.Label();
            this.textBoxEnvVars = new System.Windows.Forms.TextBox();
            this.labelEnvVars = new System.Windows.Forms.Label();
            this.groupBoxVulkanApp = new System.Windows.Forms.GroupBox();
            this.groupBoxLayer = new System.Windows.Forms.GroupBox();
            this.groupBoxVulkanApp.SuspendLayout();
            this.groupBoxLayer.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelExe
            // 
            this.labelExe.AutoSize = true;
            this.labelExe.Location = new System.Drawing.Point(45, 75);
            this.labelExe.Name = "labelExe";
            this.labelExe.Size = new System.Drawing.Size(92, 20);
            this.labelExe.TabIndex = 0;
            this.labelExe.Text = "Executable:";
            // 
            // labelOutputDir
            // 
            this.labelOutputDir.AutoSize = true;
            this.labelOutputDir.Location = new System.Drawing.Point(27, 45);
            this.labelOutputDir.Name = "labelOutputDir";
            this.labelOutputDir.Size = new System.Drawing.Size(126, 20);
            this.labelOutputDir.TabIndex = 1;
            this.labelOutputDir.Text = "Output directory:";
            // 
            // textBoxExe
            // 
            this.textBoxExe.Location = new System.Drawing.Point(226, 65);
            this.textBoxExe.Name = "textBoxExe";
            this.textBoxExe.Size = new System.Drawing.Size(662, 26);
            this.textBoxExe.TabIndex = 2;
            // 
            // textBoxOutputDir
            // 
            this.textBoxOutputDir.Location = new System.Drawing.Point(208, 40);
            this.textBoxOutputDir.Name = "textBoxOutputDir";
            this.textBoxOutputDir.Size = new System.Drawing.Size(662, 26);
            this.textBoxOutputDir.TabIndex = 3;
            // 
            // button1
            // 
            this.button1.Cursor = System.Windows.Forms.Cursors.Hand;
            this.button1.Location = new System.Drawing.Point(896, 35);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(100, 31);
            this.button1.TabIndex = 4;
            this.button1.Text = "...";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.buttonBrowseForExeClick);
            // 
            // buttonOutputDir
            // 
            this.buttonOutputDir.Cursor = System.Windows.Forms.Cursors.Hand;
            this.buttonOutputDir.Location = new System.Drawing.Point(896, 40);
            this.buttonOutputDir.Name = "buttonOutputDir";
            this.buttonOutputDir.Size = new System.Drawing.Size(100, 31);
            this.buttonOutputDir.TabIndex = 5;
            this.buttonOutputDir.Text = "...";
            this.buttonOutputDir.UseVisualStyleBackColor = true;
            this.buttonOutputDir.Click += new System.EventHandler(this.buttonBrowseForOutputFolderClick);
            // 
            // buttonLaunch
            // 
            this.buttonLaunch.Cursor = System.Windows.Forms.Cursors.Hand;
            this.buttonLaunch.Location = new System.Drawing.Point(914, 428);
            this.buttonLaunch.Name = "buttonLaunch";
            this.buttonLaunch.Size = new System.Drawing.Size(100, 37);
            this.buttonLaunch.TabIndex = 6;
            this.buttonLaunch.Text = "Launch";
            this.buttonLaunch.UseVisualStyleBackColor = true;
            this.buttonLaunch.Click += new System.EventHandler(this.buttonLaunchClick);
            // 
            // button3
            // 
            this.button3.Cursor = System.Windows.Forms.Cursors.Hand;
            this.button3.Location = new System.Drawing.Point(712, 428);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(178, 37);
            this.button3.TabIndex = 7;
            this.button3.Text = "Open output folder";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.buttonOpenDirectoryClick);
            // 
            // buttonWorkingDir
            // 
            this.buttonWorkingDir.Cursor = System.Windows.Forms.Cursors.Hand;
            this.buttonWorkingDir.Location = new System.Drawing.Point(896, 88);
            this.buttonWorkingDir.Name = "buttonWorkingDir";
            this.buttonWorkingDir.Size = new System.Drawing.Size(100, 31);
            this.buttonWorkingDir.TabIndex = 10;
            this.buttonWorkingDir.Text = "...";
            this.buttonWorkingDir.UseVisualStyleBackColor = true;
            this.buttonWorkingDir.Click += new System.EventHandler(this.buttonWorkingDir_Click);
            // 
            // textBoxWorkingDir
            // 
            this.textBoxWorkingDir.Location = new System.Drawing.Point(226, 117);
            this.textBoxWorkingDir.Name = "textBoxWorkingDir";
            this.textBoxWorkingDir.Size = new System.Drawing.Size(662, 26);
            this.textBoxWorkingDir.TabIndex = 9;
            // 
            // labelWorkingDir
            // 
            this.labelWorkingDir.AutoSize = true;
            this.labelWorkingDir.Location = new System.Drawing.Point(45, 122);
            this.labelWorkingDir.Name = "labelWorkingDir";
            this.labelWorkingDir.Size = new System.Drawing.Size(135, 20);
            this.labelWorkingDir.TabIndex = 8;
            this.labelWorkingDir.Text = "Working directory:";
            // 
            // textBoxPipelineNames
            // 
            this.textBoxPipelineNames.Location = new System.Drawing.Point(208, 91);
            this.textBoxPipelineNames.Name = "textBoxPipelineNames";
            this.textBoxPipelineNames.Size = new System.Drawing.Size(662, 26);
            this.textBoxPipelineNames.TabIndex = 12;
            // 
            // labelPipelineNames
            // 
            this.labelPipelineNames.AutoSize = true;
            this.labelPipelineNames.Location = new System.Drawing.Point(27, 95);
            this.labelPipelineNames.Name = "labelPipelineNames";
            this.labelPipelineNames.Size = new System.Drawing.Size(120, 20);
            this.labelPipelineNames.TabIndex = 11;
            this.labelPipelineNames.Text = "Pipeline names:";
            // 
            // textBoxEnvVars
            // 
            this.textBoxEnvVars.Cursor = System.Windows.Forms.Cursors.SizeWE;
            this.textBoxEnvVars.Location = new System.Drawing.Point(226, 163);
            this.textBoxEnvVars.Name = "textBoxEnvVars";
            this.textBoxEnvVars.Size = new System.Drawing.Size(662, 26);
            this.textBoxEnvVars.TabIndex = 14;
            // 
            // labelEnvVars
            // 
            this.labelEnvVars.AutoSize = true;
            this.labelEnvVars.Location = new System.Drawing.Point(45, 169);
            this.labelEnvVars.Name = "labelEnvVars";
            this.labelEnvVars.Size = new System.Drawing.Size(168, 20);
            this.labelEnvVars.TabIndex = 13;
            this.labelEnvVars.Text = "Environment variables:";
            // 
            // groupBoxVulkanApp
            // 
            this.groupBoxVulkanApp.Controls.Add(this.button1);
            this.groupBoxVulkanApp.Controls.Add(this.buttonWorkingDir);
            this.groupBoxVulkanApp.Location = new System.Drawing.Point(18, 29);
            this.groupBoxVulkanApp.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.groupBoxVulkanApp.Name = "groupBoxVulkanApp";
            this.groupBoxVulkanApp.Padding = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.groupBoxVulkanApp.Size = new System.Drawing.Size(1017, 194);
            this.groupBoxVulkanApp.TabIndex = 15;
            this.groupBoxVulkanApp.TabStop = false;
            this.groupBoxVulkanApp.Text = "Vulkan app";
            // 
            // groupBoxLayer
            // 
            this.groupBoxLayer.Controls.Add(this.textBoxOutputDir);
            this.groupBoxLayer.Controls.Add(this.buttonOutputDir);
            this.groupBoxLayer.Controls.Add(this.labelPipelineNames);
            this.groupBoxLayer.Controls.Add(this.textBoxPipelineNames);
            this.groupBoxLayer.Controls.Add(this.labelOutputDir);
            this.groupBoxLayer.Location = new System.Drawing.Point(18, 248);
            this.groupBoxLayer.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.groupBoxLayer.Name = "groupBoxLayer";
            this.groupBoxLayer.Padding = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.groupBoxLayer.Size = new System.Drawing.Size(1017, 152);
            this.groupBoxLayer.TabIndex = 16;
            this.groupBoxLayer.TabStop = false;
            this.groupBoxLayer.Text = "Layer";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(1054, 497);
            this.Controls.Add(this.textBoxEnvVars);
            this.Controls.Add(this.labelEnvVars);
            this.Controls.Add(this.textBoxWorkingDir);
            this.Controls.Add(this.labelWorkingDir);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.buttonLaunch);
            this.Controls.Add(this.textBoxExe);
            this.Controls.Add(this.labelExe);
            this.Controls.Add(this.groupBoxVulkanApp);
            this.Controls.Add(this.groupBoxLayer);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Form1";
            this.Text = "RGA Layer Launcher";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBoxVulkanApp.ResumeLayout(false);
            this.groupBoxLayer.ResumeLayout(false);
            this.groupBoxLayer.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelExe;
        private System.Windows.Forms.Label labelOutputDir;
        private System.Windows.Forms.TextBox textBoxExe;
        private System.Windows.Forms.TextBox textBoxOutputDir;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button buttonOutputDir;
        private System.Windows.Forms.Button buttonLaunch;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Button buttonWorkingDir;
        private System.Windows.Forms.TextBox textBoxWorkingDir;
        private System.Windows.Forms.Label labelWorkingDir;
        private System.Windows.Forms.TextBox textBoxPipelineNames;
        private System.Windows.Forms.Label labelPipelineNames;
        private System.Windows.Forms.TextBox textBoxEnvVars;
        private System.Windows.Forms.Label labelEnvVars;
        private System.Windows.Forms.GroupBox groupBoxVulkanApp;
        private System.Windows.Forms.GroupBox groupBoxLayer;
    }
}

