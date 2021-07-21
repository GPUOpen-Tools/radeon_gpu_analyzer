using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RGALayerLauncher
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private bool OpenExecutableDialog(string initialDir, ref string selectedFileName)
        {
            bool ret = false;
            var fileDialog = new OpenFileDialog();
            fileDialog.Filter = "Executable files (*.exe)|*.exe";
            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                selectedFileName = fileDialog.FileName;
                try
                {
                    textBoxWorkingDir.Text = System.IO.Path.GetDirectoryName(selectedFileName);
                    ret = true;
                }
                catch
                {
                    MessageBox.Show("Failed to extract working directory from Executable path.",
                        "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            return ret;
        }

        private void buttonBrowseForExeClick(object sender, EventArgs e)
        {
            string selectedFileName = "";
            string initialDir = "";
            if(System.IO.Directory.Exists(textBoxExe.Text))
            {
                initialDir = textBoxExe.Text;
            }
            bool ret = OpenExecutableDialog(initialDir, ref selectedFileName);
            if (ret)
            {
                textBoxExe.Text = selectedFileName;
            }
        }

        private void buttonBrowseForOutputFolderClick(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.RootFolder = Environment.SpecialFolder.MyComputer;
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                textBoxOutputDir.Text = dialog.SelectedPath;
            }
        }

        private void buttonLaunchClick(object sender, EventArgs e)
        {
            try
            {
                bool shouldAbort = false;

                // Validate executable.
                if (!System.IO.File.Exists(textBoxExe.Text))
                {
                    MessageBox.Show("Please specify valid executable file.",
                        "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    shouldAbort = true;
                    shouldAbort = true;
                }

                // Validate output directory.
                if (!shouldAbort && !System.IO.Directory.Exists(textBoxOutputDir.Text))
                {
                    MessageBox.Show("Please specify valid output directory.",
                        "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    shouldAbort = true;
                }

                // Validate working directory (if given).
                if (!shouldAbort && textBoxWorkingDir.Text.Length > 0 &&
                    !System.IO.Directory.Exists(textBoxWorkingDir.Text))
                {
                    MessageBox.Show("Invalid working directory.",
                        "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    shouldAbort = true;
                }

                // Validate environment variables.
                string envVars = textBoxEnvVars.Text;
                if (envVars.Length > 0)
                {
                    string[] variables = envVars.Split(';');
                    if (variables.Length == 0)
                    {
                        MessageBox.Show("Environment variables should be a semicolon-separate list.",
                            "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        shouldAbort = true;
                    }

                    if (!shouldAbort)
                    {
                        foreach (string envVar in variables)
                        {
                            const string ENV_VAR_GENERIC_ERR_MSG = "Environment variables should be a semicolon-separate list of \"Key=Value\" items.";
                            string envVarName = "";
                            string envVarValue = "";

                            if (envVar.Contains("="))
                            {
                                string[] varTokens = envVar.Split('=');
                                if (varTokens.Length != 2)
                                {
                                    MessageBox.Show(ENV_VAR_GENERIC_ERR_MSG, "Error",
                                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                                    shouldAbort = true;
                                    break;
                                }
                                else
                                {
                                    // Extract the name and value of the current environment variable.
                                    envVarName = varTokens[0];
                                    envVarValue = varTokens[1];
                                }
                            }
                            else
                            {
                                MessageBox.Show(ENV_VAR_GENERIC_ERR_MSG,
                                    "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                shouldAbort = true;
                                break;
                            }

                            shouldAbort = shouldAbort || (envVarName.Length == 0);
                            if (!shouldAbort)
                            {
                                // Set the environment variable.
                                Environment.SetEnvironmentVariable(envVarName,
                                    envVarValue, EnvironmentVariableTarget.Process);
                            }
                        }
                    }
                }

                if (!shouldAbort)
                {
                    // Enable the RGA layer.
                    Environment.SetEnvironmentVariable("ENABLE_RGA_PIPELINE_EXTRACTION_LAYER",
                        "1", EnvironmentVariableTarget.Process);

                    // Set the output directory for the RGA layer.
                    Environment.SetEnvironmentVariable("RGA_LAYER_OUTPUT_PATH",
                        textBoxOutputDir.Text, EnvironmentVariableTarget.Process);

                    // Set the specific pipeline names if requested.
                    Environment.SetEnvironmentVariable("RGA_LAYER_SPECIFIC_PIPELINE",
                        textBoxPipelineNames.Text, EnvironmentVariableTarget.Process);

                    System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
                    startInfo.FileName = textBoxExe.Text;
                    if (textBoxWorkingDir.Text.Length > 0)
                    {
                        startInfo.WorkingDirectory = textBoxWorkingDir.Text;
                    }
                    if (textBoxCommandArgs.Text.Length > 0)
                    {
                        startInfo.Arguments = textBoxCommandArgs.Text;
                    }

                    // Launch the executable.
                    System.Diagnostics.Process.Start(startInfo);
                }
            }
            catch
            {
                MessageBox.Show("Error launching the target executable.",
                    "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void buttonOpenDirectoryClick(object sender, EventArgs e)
        {
            if (!System.IO.Directory.Exists(textBoxOutputDir.Text))
            {
                MessageBox.Show("Please specify valid output directory.",
                    "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
            {
                System.Diagnostics.Process.Start(textBoxOutputDir.Text);
            }
        }

        private void buttonWorkingDir_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                textBoxWorkingDir.Text = dialog.SelectedPath;
            }
        }
    }
}
