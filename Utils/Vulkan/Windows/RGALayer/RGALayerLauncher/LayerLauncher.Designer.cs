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
            this.textBoxCommandArgs = new System.Windows.Forms.TextBox();
            this.labelCommandArgs = new System.Windows.Forms.Label();
            this.groupBoxVulkanApp.SuspendLayout();
            this.groupBoxLayer.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelExe
            // 
            this.labelExe.AutoSize = true;
            this.labelExe.Location = new System.Drawing.Point(40, 60);
            this.labelExe.Name = "labelExe";
            this.labelExe.Size = new System.Drawing.Size(81, 17);
            this.labelExe.TabIndex = 0;
            this.labelExe.Text = "Executable:";
            // 
            // labelOutputDir
            // 
            this.labelOutputDir.AutoSize = true;
            this.labelOutputDir.Location = new System.Drawing.Point(24, 36);
            this.labelOutputDir.Name = "labelOutputDir";
            this.labelOutputDir.Size = new System.Drawing.Size(114, 17);
            this.labelOutputDir.TabIndex = 1;
            this.labelOutputDir.Text = "Output directory:";
            // 
            // textBoxExe
            // 
            this.textBoxExe.Location = new System.Drawing.Point(201, 52);
            this.textBoxExe.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxExe.Name = "textBoxExe";
            this.textBoxExe.Size = new System.Drawing.Size(589, 22);
            this.textBoxExe.TabIndex = 2;
            // 
            // textBoxOutputDir
            // 
            this.textBoxOutputDir.Location = new System.Drawing.Point(185, 32);
            this.textBoxOutputDir.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxOutputDir.Name = "textBoxOutputDir";
            this.textBoxOutputDir.Size = new System.Drawing.Size(589, 22);
            this.textBoxOutputDir.TabIndex = 3;
            // 
            // button1
            // 
            this.button1.Cursor = System.Windows.Forms.Cursors.Hand;
            this.button1.Location = new System.Drawing.Point(796, 28);
            this.button1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(89, 25);
            this.button1.TabIndex = 4;
            this.button1.Text = "...";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.buttonBrowseForExeClick);
            // 
            // buttonOutputDir
            // 
            this.buttonOutputDir.Cursor = System.Windows.Forms.Cursors.Hand;
            this.buttonOutputDir.Location = new System.Drawing.Point(796, 32);
            this.buttonOutputDir.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonOutputDir.Name = "buttonOutputDir";
            this.buttonOutputDir.Size = new System.Drawing.Size(89, 25);
            this.buttonOutputDir.TabIndex = 5;
            this.buttonOutputDir.Text = "...";
            this.buttonOutputDir.UseVisualStyleBackColor = true;
            this.buttonOutputDir.Click += new System.EventHandler(this.buttonBrowseForOutputFolderClick);
            // 
            // buttonLaunch
            // 
            this.buttonLaunch.Cursor = System.Windows.Forms.Cursors.Hand;
            this.buttonLaunch.Location = new System.Drawing.Point(812, 370);
            this.buttonLaunch.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonLaunch.Name = "buttonLaunch";
            this.buttonLaunch.Size = new System.Drawing.Size(89, 30);
            this.buttonLaunch.TabIndex = 6;
            this.buttonLaunch.Text = "Launch";
            this.buttonLaunch.UseVisualStyleBackColor = true;
            this.buttonLaunch.Click += new System.EventHandler(this.buttonLaunchClick);
            // 
            // button3
            // 
            this.button3.Cursor = System.Windows.Forms.Cursors.Hand;
            this.button3.Location = new System.Drawing.Point(633, 370);
            this.button3.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(158, 30);
            this.button3.TabIndex = 7;
            this.button3.Text = "Open output folder";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.buttonOpenDirectoryClick);
            // 
            // buttonWorkingDir
            // 
            this.buttonWorkingDir.Cursor = System.Windows.Forms.Cursors.Hand;
            this.buttonWorkingDir.Location = new System.Drawing.Point(796, 70);
            this.buttonWorkingDir.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.buttonWorkingDir.Name = "buttonWorkingDir";
            this.buttonWorkingDir.Size = new System.Drawing.Size(89, 25);
            this.buttonWorkingDir.TabIndex = 10;
            this.buttonWorkingDir.Text = "...";
            this.buttonWorkingDir.UseVisualStyleBackColor = true;
            this.buttonWorkingDir.Click += new System.EventHandler(this.buttonWorkingDir_Click);
            // 
            // textBoxWorkingDir
            // 
            this.textBoxWorkingDir.Location = new System.Drawing.Point(201, 94);
            this.textBoxWorkingDir.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxWorkingDir.Name = "textBoxWorkingDir";
            this.textBoxWorkingDir.Size = new System.Drawing.Size(589, 22);
            this.textBoxWorkingDir.TabIndex = 9;
            // 
            // labelWorkingDir
            // 
            this.labelWorkingDir.AutoSize = true;
            this.labelWorkingDir.Location = new System.Drawing.Point(40, 98);
            this.labelWorkingDir.Name = "labelWorkingDir";
            this.labelWorkingDir.Size = new System.Drawing.Size(123, 17);
            this.labelWorkingDir.TabIndex = 8;
            this.labelWorkingDir.Text = "Working directory:";
            // 
            // textBoxPipelineNames
            // 
            this.textBoxPipelineNames.Location = new System.Drawing.Point(185, 73);
            this.textBoxPipelineNames.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxPipelineNames.Name = "textBoxPipelineNames";
            this.textBoxPipelineNames.Size = new System.Drawing.Size(589, 22);
            this.textBoxPipelineNames.TabIndex = 12;
            // 
            // labelPipelineNames
            // 
            this.labelPipelineNames.AutoSize = true;
            this.labelPipelineNames.Location = new System.Drawing.Point(24, 76);
            this.labelPipelineNames.Name = "labelPipelineNames";
            this.labelPipelineNames.Size = new System.Drawing.Size(108, 17);
            this.labelPipelineNames.TabIndex = 11;
            this.labelPipelineNames.Text = "Pipeline names:";
            // 
            // textBoxEnvVars
            // 
            this.textBoxEnvVars.Cursor = System.Windows.Forms.Cursors.SizeWE;
            this.textBoxEnvVars.Location = new System.Drawing.Point(201, 169);
            this.textBoxEnvVars.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxEnvVars.Name = "textBoxEnvVars";
            this.textBoxEnvVars.Size = new System.Drawing.Size(589, 22);
            this.textBoxEnvVars.TabIndex = 14;
            // 
            // labelEnvVars
            // 
            this.labelEnvVars.AutoSize = true;
            this.labelEnvVars.Location = new System.Drawing.Point(40, 174);
            this.labelEnvVars.Name = "labelEnvVars";
            this.labelEnvVars.Size = new System.Drawing.Size(152, 17);
            this.labelEnvVars.TabIndex = 13;
            this.labelEnvVars.Text = "Environment variables:";
            // 
            // groupBoxVulkanApp
            // 
            this.groupBoxVulkanApp.Controls.Add(this.textBoxCommandArgs);
            this.groupBoxVulkanApp.Controls.Add(this.button1);
            this.groupBoxVulkanApp.Controls.Add(this.labelCommandArgs);
            this.groupBoxVulkanApp.Controls.Add(this.buttonWorkingDir);
            this.groupBoxVulkanApp.Location = new System.Drawing.Point(16, 23);
            this.groupBoxVulkanApp.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBoxVulkanApp.Name = "groupBoxVulkanApp";
            this.groupBoxVulkanApp.Padding = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBoxVulkanApp.Size = new System.Drawing.Size(904, 185);
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
            this.groupBoxLayer.Location = new System.Drawing.Point(16, 226);
            this.groupBoxLayer.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBoxLayer.Name = "groupBoxLayer";
            this.groupBoxLayer.Padding = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBoxLayer.Size = new System.Drawing.Size(904, 122);
            this.groupBoxLayer.TabIndex = 16;
            this.groupBoxLayer.TabStop = false;
            this.groupBoxLayer.Text = "Layer";
            // 
            // textBoxCommandArgs
            // 
            this.textBoxCommandArgs.Cursor = System.Windows.Forms.Cursors.SizeWE;
            this.textBoxCommandArgs.Location = new System.Drawing.Point(185, 108);
            this.textBoxCommandArgs.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.textBoxCommandArgs.Name = "textBoxCommandArgs";
            this.textBoxCommandArgs.Size = new System.Drawing.Size(589, 22);
            this.textBoxCommandArgs.TabIndex = 18;
            // 
            // labelCommandArgs
            // 
            this.labelCommandArgs.AutoSize = true;
            this.labelCommandArgs.Location = new System.Drawing.Point(24, 113);
            this.labelCommandArgs.Name = "labelCommandArgs";
            this.labelCommandArgs.Size = new System.Drawing.Size(80, 17);
            this.labelCommandArgs.TabIndex = 17;
            this.labelCommandArgs.Text = "Arguments:";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(937, 421);
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
            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.Name = "Form1";
            this.Text = "RGA Layer Launcher";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBoxVulkanApp.ResumeLayout(false);
            this.groupBoxVulkanApp.PerformLayout();
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
        private System.Windows.Forms.TextBox textBoxCommandArgs;
        private System.Windows.Forms.Label labelCommandArgs;
    }
}

