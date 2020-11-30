using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;

namespace DXR_State_Editor
{
    public partial class MainWindow : Window
    {
        private void LoadJSON(String queryTitle)
        {
            MessageBoxResult userDeleteAllMacroElements = MessageBoxResult.None;
            bool nonDefaultJsonAssociationFound = QueryJsonForNonDefaultAssociations();
            if (nonDefaultJsonAssociationFound)
            {
                userDeleteAllMacroElements = ResetMacroElementsQuery(queryTitle);
                if (userDeleteAllMacroElements == MessageBoxResult.No)
                {
                    // User has canceled loading file
                    return;
                }
            }
            else
            {
                // The App is in the default state. We must clear the default elements such that
                // only the file state is reflected.
                RemoveDefaultUserLevelElements();
            }

            // Import data from file
            OpenFileDialog fileDialog = new OpenFileDialog();
            fileDialog.Multiselect = true;
            Nullable<bool> result = fileDialog.ShowDialog();
            String[] files = null;
            if (result.Value)
            {
                files = fileDialog.FileNames;
            }

            if (files != null && files.Length > 0)
            {
                List<JsonFileData> jsonFileDataList = new List<JsonFileData>();
                ReadJSONFileContents(files, jsonFileDataList);
                if (jsonFileDataList.Count > 0)
                {
                    if (userDeleteAllMacroElements == MessageBoxResult.Yes)
                    {
                        RemoveAllMacroElementGroups();
                    }

                    ConstructMacroElementsFromJSON(jsonFileDataList);
                }
            }

            // Determine quick description visibility and content
            DetermineQuickDescriptionVisibilityAndContentByApplicationMode(CurrentApplicationMode);
        }

        private void HandleResetJSONContextMenu(object sender, RoutedEventArgs e)
        {
            MessageBoxResult userDeleteAllMacroElements = MessageBoxResult.None;
            bool nonDefaultJsonAssociationFound = QueryJsonForNonDefaultAssociations();
            if (nonDefaultJsonAssociationFound)
            {
                userDeleteAllMacroElements = ResetMacroElementsQuery("Reset All Data?");
                if (userDeleteAllMacroElements == MessageBoxResult.Yes)
                {
                    RemoveAllMacroElementGroups();
                    InitializeDefaultUserLevelElements();
                    DetermineQuickDescriptionVisibilityAndContentByApplicationMode(CurrentApplicationMode);

                    // Pipeline config has default state, but would not be in an accessible list as there
                    // is only one instance of this MacroElement. We must clean this state manually.
                    Mode previousMode = CurrentApplicationMode;
                    SetApplicationMode(Mode.RaytracingPipeline);
                    TextBox MaxTraceRecursionDepthTextBox = FindName("RaytracingPipelineMaxTraceRecursionDepthTextBox0") as TextBox;
                    if (MaxTraceRecursionDepthTextBox == null)
                    {
                        postErrorMessage("A call to " + nameof(HandleResetJSONContextMenu) +
                            " was unable to acquire the max trace recursion depth text box");
                        return;
                    }
                    MaxTraceRecursionDepthTextBox.Text = "1";

                    TextBox ExportsTextBox = FindName("RaytracingPipelineExportsTextBox0") as TextBox;
                    if (ExportsTextBox == null)
                    {
                        postErrorMessage("A call to " + nameof(HandleResetJSONContextMenu) +
                            " was unable to acquire the exports text box");
                        return;
                    }
                    ExportsTextBox.Text = null;
                    SetApplicationMode(previousMode);
                }
            }

            // Restore welcome screen
            SetApplicationMode(Mode.Welcome);
        }

        private void HandleLoadJSONButton(object sender, RoutedEventArgs e)
        {
            try
            {
                Button buSender = sender as Button;
                if (buSender == null)
                {
                    postErrorMessage("A call to " + nameof(HandleLoadJSONButton) +
                        "received a null Button object.");
                    return;
                }

                String queryTitle = buSender.Tag as String;
                if (queryTitle == null)
                {
                    postErrorMessage("A call to " + nameof(HandleLoadJSONButton) +
                        "received a null queryTitle string [ tag ].");
                    return;
                }

                // Note: state clearing is handled in LoadJSON();
                LoadJSON(queryTitle);
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to load JSON files via the file browse button: " +
                    ex.ToString());
            }
        }

        public void BrowseForFilePath(object sender, RoutedEventArgs e)
        {
            try
            {
                Button senderButton = sender as Button;
                if (senderButton == null)
                {
                    postErrorMessage("A call to " + nameof(BrowseForFilePath) + " was initaited from an unauthorized source.");
                    return;
                }

                String textBoxName = senderButton.Tag as String;
                if ((textBoxName == null) || (textBoxName.Length == 0))
                {
                    postErrorMessage("A call to " + nameof(BrowseForFilePath) + " was unable to find the target TextBox ID string.");
                    return;
                }

                TextBox targetTextBox = FindName(textBoxName) as TextBox;
                if (targetTextBox == null)
                {
                    postErrorMessage("A call to " + nameof(BrowseForFilePath) + " was unable to match an ID string with a valid TextBox.");
                }

                OpenFileDialog fileDialog = new OpenFileDialog();
                Nullable<bool> result = fileDialog.ShowDialog();
                if (result.Value)
                {
                    targetTextBox.Text = fileDialog.FileName;
                }
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to browse for a file path: " +
                    ex.ToString());
            }
        }

        private void AddButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                // Ensure that the expected object is initiating this call.
                Button addButton = sender as Button;
                if (addButton != null && addButton.Name == "addButton")
                {
                    CreateMacroElementByCurrentApplicationMode();
                }
                else
                {
                    postErrorMessage("A call to " + nameof(AddButton_Click) + " originated from an unauthorized source");
                }

                DetermineQuickDescriptionVisibilityAndContentByApplicationMode(CurrentApplicationMode);
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to handle the 'add' button click event: " +
                    ex.ToString());
            }
        }

        private MessageBoxResult ResetMacroElementsQuery(String queryTitle)
        {
            if (queryTitle == null)
            {
                postErrorMessage("A call to " + nameof(ResetMacroElementsQuery) +
                    " received a null queryTitle string.");
                return MessageBoxResult.None;
            }

            return MessageBox.Show("This will clear all data from your current workspace. Is this okay?",
                queryTitle,
                MessageBoxButton.YesNo,
                MessageBoxImage.Warning);
        }

        private void setButtonStyleSelected(Button buSender)
        {
            try
            {
                if (buSender == null)
                {
                    postWarningMessage("Unable to style button on click event. Cannot cast sender to button.");
                    return;
                }

                Style selectedStyle = FindResource("selectedButtonStyle") as Style;
                if (selectedStyle == null)
                {
                    postWarningMessage("Unable to style button on click event. Cannot obtain style definition.");
                    return;
                }

                buSender.Style = selectedStyle;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to re-style buttons: " +
                    ex.ToString());
            }
        }

        private void ShaderButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SetApplicationMode(Mode.Shader);
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to handle the 'add shader' button click event: " +
                    ex.ToString());
            }
        }

        private void HitGroupsButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SetApplicationMode(Mode.HitGroup);
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to handle the 'add hit group' button click event: " +
                    ex.ToString());
            }
        }

        private void LocalRootSignaturesButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SetApplicationMode(Mode.LocalRootSignature);
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to handle the 'add local root signature' button click event: " +
                    ex.ToString());
            }
        }

        private void GlobalRootSignaturesButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SetApplicationMode(Mode.GlobalRootSignature);
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to handle the 'add global root signature' button click event: " +
                    ex.ToString());
            }
        }

        private void RaytracingPipelineConfigButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SetApplicationMode(Mode.RaytracingPipeline);
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caugh while attempting to handle the 'add raytraching pipeline' button click eventt: " +
                    ex.ToString());
            }
        }

        private void ShaderPipelineConfigButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SetApplicationMode(Mode.ShaderPipeline);
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to handle the 'add shader pipeline/config' button click event: " +
                    ex.ToString());
            }
        }

        private void SaveJSONAsRPSO()
        {
            TextBox jsonTextBox = FindName("jsonViewText") as TextBox;
            if (jsonTextBox == null)
            {
                postErrorMessage("A call to " + nameof(SaveJSONAsRPSO) + " was unable to aquire the target text.");
            }

            String validationstring = JSONInputIsValid();
            if (validationstring == null)
            {
                postErrorMessage("A call to " + nameof(SaveJSONAsRPSO) + " was unable to aquire a validation string.");
                return;
            }

            if (validationstring.Length > 0)
            {
                String msgString =  validationstring +
                    "\nAre you sure you want to ignore this warning and save the file?";

                MessageBoxResult msgBoxResult = MessageBox.Show(msgString,
                    "Invalid JSON State Detected",
                    MessageBoxButton.YesNo,
                    MessageBoxImage.Warning);

                if (msgBoxResult != MessageBoxResult.Yes)
                {
                    return;
                }
            }

            // [ cfarvin::REMOVE ]

            SaveFileDialog fileDialog = new SaveFileDialog()
            {
                FileName = "DXRState",
                DefaultExt = ".rpso"
            };

            Nullable<bool> result = fileDialog.ShowDialog();
            if (result.Value)
            {
                System.IO.File.WriteAllText(@fileDialog.FileName, jsonTextBox.Text);
            }
        }

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SaveJSONAsRPSO();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to handle the 'save' button click event: " +
                    ex.ToString());
            }
        }

        private void ClipboardButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox jsonTextBox = FindName("jsonViewText") as TextBox;
                if (jsonTextBox == null)
                {
                    postErrorMessage(" A call to " + nameof(ClipboardButton_Click) +
                        " could not aquire the text source.");
                    return;
                }

                Clipboard.SetText(jsonTextBox.Text);
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to handle the 'copy to clipboard' button click event: " +
                    ex.ToString());
            }
        }
    }
}