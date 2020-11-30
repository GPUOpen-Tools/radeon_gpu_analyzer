using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
namespace DXR_State_Editor
{
    //
    // Macro Element Definition
    //
    public struct MacroElement
    {
        public Mode applicationMode;
        public FrameworkElement rootElement;
        public Panel targetElement;
        public List<String> regiteredChildrenNames;
        public bool hasItemSeperator;
    }
    public class DXRScrollViewer : ScrollViewer
    {
        public enum ScrollCommandOrigin
        {
            UserInitiated,
            SystemInitiated
        };
        public bool shouldAutoScroll = true;
        public ScrollCommandOrigin scrollCommandOrigin = ScrollCommandOrigin.UserInitiated;
    }
    public partial class MainWindow : Window
    {
        readonly String welcomeText = "Select a pipeline component on the left to start building your DXR state object," +
        //Environment.NewLine + "or drag and drop a valid DXR state JSON file onto the button bar at the bottom " +
        Environment.NewLine + "or use the 'Load JSON' button in the bottom right of the window.";
        // Master Macro Element List
        public List<MacroElement> Master_MacroElementList = new List<MacroElement>();
        // Content View Settings
        public class ContentViewSettings
        {
            public const double ContentMinWidth = 950;
            public const double ContentItemHeight = 35;
            public const double ContentTextBoxMinWidth = 350;
            public const double ContentTextBoxMaxWidth = 600;
            public const double ContentTextBoxMargin_left = 5;
            public const double ContentTextBoxMargin_top = 5;
            public const double ContentTextBoxMargin_right = 5;
            public const double ContentTextBoxMargin_bottom = 20;
            public const double ContentComboBoxMinWidth = 350;
            public const double ContentComboBoxMargin_left = 5;
            public const double ContentComboBoxMargin_top = 5;
            public const double ContentComboBoxMargin_right = 5;
            public const double ContentComboBoxMargin_bottom = 20;
            public const double ContentBrowseButtonWidth = 80;
            public const double ContentBrowseButtonMargin_left = 5;
            public const double ContentBrowseButtonMargin_top = 5;
            public const double ContentBrowseButtonMargin_right = 5;
            public const double ContentBrowseButtonMargin_bottom = 20;
            public const double ContentRemoveButton_withBrowse_withTextBox_MarginLeft = ContentTextBoxMargin_right;
            public const double ContentRemoveButton_withBrowse_withTextBox_MarginBottom = 20;
            public const double ContentRemoveButton_withoutBrowse_withTextBox_MarginLeft =
                ContentTextBoxMargin_right + ContentBrowseButtonWidth + ContentBrowseButtonMargin_left + ContentBrowseButtonMargin_right;
            public const double ContentRemoveButton_withoutBrowse_withTextBox_MarginBottom = 20;
        };
        const String ConstStringBinaryDXILType = "Binary (DXIL)";
        const String ConstStringBinaryType = "Binary";
        const String ConstStringHLSLType = "HLSL";
        const String defaultYetInvalidFilePathString = "<File Path Required>";
        const String shaderFilePathToolTipString = "Full path to the shader on your device.";
        const String shaderEntryPointToolTipString = "Accepts a comma separated list of entry points in the following forms:\n\n"
                 + "\"entry_point_1, entry_point_2, entry_point_3\"\n"
                 + "or\n"
                 + "\"<entry_point_1, link_name_1>, <entry_point_2, link_name_2>\"\n"
                 + "or an unordered combination of these, such as:\n"
                 + "\"<entry_point_1, link_name_1>, entry_point_2, <entry_point_3, link_name_3>\"\n\n"
                 + "The text will be colored red if an invalid entry is present.";
        const String hitGroupNameToolTipString = "A name for this hit group.";
        const String hitGroupIntersectionShaderToolTipString = "A name for this hit group's intersection shader.";
        const String hitGroupAnyHitShaderToolTipString = "A name for this hit group's any hit shader.";
        const String hitGroupClosestHitShaderToolTipString = "A name for this hit group's closest hit shader.";
        const String localRootSignaturesFilePathToolTipString = "The full path to this local root signature.";
        const String localRootSignaturesMacroNameShaderToolTipString = "A name for this local root signature's macro (HLSL).";
        const String localRootSignaturesExportsToolTipString = "A comma separated list of exports to be associated with this local root signature.";
        const String globalRootSignaturesFilePathToolTipString = "The full path to this global root signature.";
        const String globalRootSignaturesMacroNameShaderToolTipString = "A name for this global root signature's macro (HLSL).";
        const String globalRootSignaturesExportsToolTipString = "A comma separated list of exports to be associated with this global root signature.";
        const String RaytracingPipelineMaxTraceRecursionDepthToolTipString = "The max trace recursion depth for this raytracing pipeline (1 means no recursion).";
        const String RaytracingPipelineExportsToolTipString = "A comma separated list of exports for this raytracing pipeline.";
        const String ShaderPipelinePayloadSizeToolTipString = "The maximum storage for scalars in ray payloads.";
        const String ShaderPipelinesMaxAttributeSizeToolTipString = "The maximum number of bytes that can be used for attributes in pipelines containing the relevant shader.";
        const String ShaderPipelinesExportsToolTipString = "A comma separated list of exports to be associated with this shader config element.";

        public void InvalidateGeneralTextBox(TextBox targetTextBox)
        {
            if (targetTextBox == null)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was unable to aquire the target label.");
                return;
            }
            Style errorStyle = FindResource("errorTextBoxStyle") as Style;
            if (errorStyle == null)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was unable to aquire the target style.");
                return;
            }
            targetTextBox.Style = errorStyle;
        }
        public void InvalidateGeneralTextBox(TextBox targetTextBox, String toolTip)
        {
            if (targetTextBox == null)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was unable to aquire the target label.");
                return;
            }
            Style errorStyle = FindResource("errorTextBoxStyle") as Style;
            if (errorStyle == null)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was unable to aquire the target style.");
                return;
            }
            if (toolTip == null || toolTip.Length < 1)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was initaited with a null or empty tool tip string.");
                return;
            }
            targetTextBox.Style = errorStyle;
            targetTextBox.ToolTip = toolTip;
        }
        public void InvalidateGeneralTextBox(TextBox targetTextBox,
            String toolTip,
            List<TextBox> invalidatedTextboxes)
        {
            if (targetTextBox == null)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was unable to aquire the target label.");
                return;
            }
            Style errorStyle = FindResource("errorTextBoxStyle") as Style;
            if (errorStyle == null)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was unable to aquire the target style.");
                return;
            }
            if (toolTip == null || toolTip.Length < 1)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was initaited with a null or empty tool tip string.");
                return;
            }
            if (invalidatedTextboxes == null)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was initaited with a null invalidation List<TextBox>.");
                return;
            }
            targetTextBox.Style = errorStyle;
            targetTextBox.ToolTip = toolTip;
            if (!invalidatedTextboxes.Contains(targetTextBox))
            {
                invalidatedTextboxes.Add(targetTextBox);
            }
        }
        public void ClearDefaultFilePathText(object sender, RoutedEventArgs e)
        {
            if (sender == null)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was passed a null sender object.");
                return;
            }
            TextBox targetTextBox = sender as TextBox;
            if (targetTextBox == null)
            {
                postErrorMessage("A call to " + nameof(InvalidateGeneralTextBox) +
                    " was unable to aquire the target label.");
                return;
            }
            if (targetTextBox.Text == defaultYetInvalidFilePathString)
            {
                targetTextBox.Text = "";
            }
        }
        public static bool ValidateMacroElement(MacroElement macroElement)
        {
            bool isValid = true;
            if (macroElement.targetElement == null || macroElement.targetElement == null)
            {
                isValid = false;
            }
            return isValid;
        }
        public void HandleDXRScrollViewerScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            DXRScrollViewer scrollViewer = sender as DXRScrollViewer;
            if (scrollViewer == null)
            {
                postWarningMessage("A call to " + nameof(HandleDXRScrollViewerScrollChanged) +
                    " was unable to acquire the DXRScrollViewer.");
                return;
            }
            // Carry out user initiated scrolling and determine value of shouldAutoScroll member.
            if (scrollViewer.scrollCommandOrigin == DXRScrollViewer.ScrollCommandOrigin.UserInitiated)
            {
                if (scrollViewer.VerticalOffset >= (0.98 * scrollViewer.ScrollableHeight))
                {
                    scrollViewer.shouldAutoScroll = true;
                }
                else
                {
                    scrollViewer.shouldAutoScroll = false;
                }
            }
        }

        public void SetComboBoxItemContent(ComboBox cBox, List<String> comboBoxContentTextList)
        {
            bool shouldContinue = true;
            if (cBox == null)
            {
                shouldContinue = false;
                postErrorMessage("A call to " + nameof(SetComboBoxItemContent) +
                    " received a null ComboBox.");
            }
            else if (comboBoxContentTextList == null)
            {
                shouldContinue = false;
                postErrorMessage("A call to " + nameof(SetComboBoxItemContent) +
                    " received a null ComboBox.");
            }

            if (shouldContinue)
            {
                foreach (String contentString in comboBoxContentTextList)
                {
                    if (contentString != null &&
                        contentString.Length > 0)
                    {
                        ComboBoxItem cbItem = new ComboBoxItem();
                        cbItem.Content = contentString;
                        cbItem.Tag = contentString;
                        cBox.Items.Add(cbItem);
                        cbItem.Cursor = Cursors.Hand;
                    }
                    else
                    {
                        postWarningMessage("A call to " + nameof(SetComboBoxItemContent) +
                            " was unable to add a content string to a ComboBoxItem widget.");
                    }
                }
            }
        }

        public void RemoveMacroElement(MacroElement macroElement)
        {
            bool shouldContinue = true;
            if (macroElement.applicationMode == Mode.Welcome ||
                macroElement.applicationMode == Mode.GeneralStateConfig ||
                macroElement.applicationMode == Mode.RaytracingPipeline ||
                macroElement.applicationMode == Mode.Output ||
                macroElement.applicationMode == Mode.ApplicationError)
            {
                shouldContinue = false;
            }
            Grid cGrid = FindName("contentGrid") as Grid;
            if (cGrid == null)
            {
                postErrorMessage(" A call to " + nameof(RemoveMacroElement) +
                    " failed to aquire the main content grid.");
                shouldContinue = false;
            }
            Grid rootGrid = macroElement.rootElement as Grid;
            if (rootGrid == null)
            {
                postErrorMessage(" A call to " + nameof(RemoveMacroElement)
                    + " failed to aquire the root of the target MacroElement.");
                shouldContinue = false;
            }
            if (shouldContinue)
            {
                // Unregister all associated names.
                foreach (String name in macroElement.regiteredChildrenNames)
                {
                    UnregisterName(name);
                }
                // Destroy all children items.
                rootGrid.Children.Clear();
                cGrid.Children.Remove(rootGrid);
                // Correct mode count.
                if (GetModeCount(macroElement.applicationMode) > 0)
                {
                    DecrementModeCount(macroElement.applicationMode);
                }
                // Remove the associated data from the JSON register lists
                // The Global JSON view wil be updated by RemoveJSONDataByID()
                RemoveJSONDataByID(macroElement.rootElement.Tag as String,
                    macroElement.applicationMode);
                // Apply the correct macro separator visibility by mode count.
                SetMacroElementSeparatorVisibilityByModeCount();
                // Remove from master list
                Master_MacroElementList.Remove(macroElement);
            }
        }
        public void RemoveAllMacroElementGroups()
        {
            try
            {
                Grid cGrid = FindName("contentGrid") as Grid;
                if (cGrid == null)
                {
                    postErrorMessage(" A call to " + nameof(RemoveAllMacroElementGroups) +
                        " failed to aquire the main content grid.");
                    return;
                }
                List<MacroElement> removalQueue = new List<MacroElement>();
                for (int index = 0; index < Master_MacroElementList.Count; index++)
                {
                    MacroElement macroElement = Master_MacroElementList[index];
                    if (macroElement.applicationMode == Mode.Welcome ||
                        macroElement.applicationMode == Mode.GeneralStateConfig ||
                        macroElement.applicationMode == Mode.RaytracingPipeline ||
                        macroElement.applicationMode == Mode.Output ||
                        macroElement.applicationMode == Mode.ApplicationError)
                    {
                        continue;
                    }
                    removalQueue.Add(macroElement);
                    Grid rootGrid = macroElement.rootElement as Grid;
                    if (rootGrid == null)
                    {
                        postErrorMessage(" A call to " + nameof(RemoveAllMacroElementGroups)
                            + " failed to aquire the root of the target MacroElement.");
                        return;
                    }
                    // Unregister all associated names.
                    foreach (String name in macroElement.regiteredChildrenNames)
                    {
                        UnregisterName(name);
                    }
                    rootGrid.Children.Clear();
                    cGrid.Children.Remove(rootGrid);
                    // Correct mode count
                    if (GetModeCount(macroElement.applicationMode) > 0)
                    {
                        DecrementModeCount(macroElement.applicationMode);
                    }
                    // Remove the associated data from the JSON register lists
                    // The Global JSON view wil be updated by RemoveJSONDataByID()
                    RemoveJSONDataByID(macroElement.rootElement.Tag as String,
                        macroElement.applicationMode);
                    // Apply the correct macro separator visibility by mode count.
                    SetMacroElementSeparatorVisibilityByModeCount();
                }
                foreach (MacroElement macroElement in removalQueue)
                {
                    Master_MacroElementList.Remove(macroElement);
                }
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to remove all macro element groups: " +
                    ex.ToString());
            }
        }
        public void HandleTrashcanIconClick(object sender, RoutedEventArgs e)
        {
            try
            {
                Button buSender = sender as Button;
                if (buSender == null)
                {
                    postErrorMessage(" A call to " + nameof(HandleTrashcanIconClick)
                        + " failed to cast the sending object as required.");
                    return;
                }
                String macroElementID = buSender.Tag as String;
                if (macroElementID == null)
                {
                    postErrorMessage(" A call to " + nameof(HandleTrashcanIconClick)
                        + " failed to aquire the necessary metadata to remove a MacroElement.");
                    return;
                }
                Grid cGrid = FindName("contentGrid") as Grid;
                if (cGrid == null)
                {
                    postErrorMessage(" A call to " + nameof(HandleTrashcanIconClick)
                        + " failed to aquire the main content grid.");
                    return;
                }
                Grid toRemove = FindName(macroElementID) as Grid;
                if (toRemove == null)
                {
                    postErrorMessage(" A call to " + nameof(HandleTrashcanIconClick)
                        + " failed to aquire the root of the target MacroElement.");
                    return;
                }
                // Use JSON representation to tell if the user has entered any data for a mode.
                // If they have, ask if they are sure before proceeding.
                if (CheckJSONAssociationByElementIDAndApplicationMode(macroElementID, CurrentApplicationMode))
                {
                    MessageBoxResult msgBoxResult = MessageBox.Show("Are you sure you want to remove this group?",
                        "JSON Data Association Detected",
                        MessageBoxButton.YesNo,
                        MessageBoxImage.Warning);
                    if (msgBoxResult == MessageBoxResult.No)
                    {
                        return;
                    }
                }
                bool foundElement = false;
                MacroElement macroElement;
                int toRemoveHashCode = toRemove.GetHashCode();
                for (int index = 0; index < Master_MacroElementList.Count; index++)
                {
                    macroElement = Master_MacroElementList[index];
                    if (macroElement.rootElement.GetHashCode() == toRemoveHashCode)
                    {
                        // Unregister all associated names.
                        foreach (String name in macroElement.regiteredChildrenNames)
                        {
                            UnregisterName(name);
                        }
                        // Remove from global element list.
                        Master_MacroElementList.RemoveAt(index);
                        foundElement = true;
                        break;
                    }
                }
                if (!foundElement)
                {
                    postWarningMessage("A call to " + nameof(HandleTrashcanIconClick) + " attempted to remove an element not registered in the global element list.");
                }
                toRemove.Children.Clear();
                cGrid.Children.Remove(toRemove);
                if (GetModeCount(CurrentApplicationMode) > 0)
                {
                    DecrementModeCount(CurrentApplicationMode);
                }
                // Remove the associated data from the JSON register lists
                // The Global JSON view wil be updated by RemoveJSONDataByID()
                RemoveJSONDataByID(macroElementID, CurrentApplicationMode);
                // Apply the correct macro separator visibility by mode count.
                SetMacroElementSeparatorVisibilityByModeCount();
                // Determine quick description visibility and content
                DetermineQuickDescriptionVisibilityAndContentByApplicationMode(CurrentApplicationMode);
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to handle a 'trash can' icon click: " +
                    ex.ToString());
            }
        }
        private MacroElement CreateMacroElementByApplicationMode(Mode mode)
        {
            // Ensure tha the application is in a valid mode to initiate this call.
            if (mode == Mode.Welcome ||
                mode == Mode.GeneralStateConfig ||
                mode == Mode.RaytracingPipeline ||
                mode == Mode.ApplicationError)
            {
                postErrorMessage("A call to " +
                    nameof(CreateMacroElementByApplicationMode) + " was issued from an invalid application mode: " + mode);
                return new MacroElement();
            }
            MacroElement macroElement = new MacroElement();
            UInt32 modeCount = GetModeCount(mode);
            switch (mode)
            {
                case Mode.Shader:
                    macroElement = CreateShaderConfig(modeCount);
                    break;
                case Mode.HitGroup:
                    macroElement = CreateHitGroupConfig(modeCount);
                    break;
                case Mode.LocalRootSignature:
                    macroElement = CreateLocalRootSignatureConfig(modeCount);
                    break;
                case Mode.GlobalRootSignature:
                    macroElement = CreateGlobalRootSignatureConfig(modeCount);
                    break;
                case Mode.ShaderPipeline:
                    macroElement = CreateShaderPipelineConfig(modeCount);
                    break;
            }
            if (ValidateMacroElement(macroElement))
            {
                // Add the newly created element to the GUI
                macroElement.targetElement.Children.Add(macroElement.rootElement);
                // Register the newly created element in the global list.
                Master_MacroElementList.Add(macroElement);
            }
            else
            {
                postErrorMessage("A call to " +
                    nameof(CreateMacroElementByApplicationMode) + " found an invalid MacroElement of mode: " + mode);
                return new MacroElement();
            }
            // If the target element was a grid, clean up the rows.
            Grid targetGrid = macroElement.targetElement as Grid;
            if (targetGrid != null)
            {
                CleanUpGridRows(targetGrid, mode);
            }
            String[] dxrScrollViewerRegisteredNames =
            {
                "generalStateConfigScrollViewer",
                "jsonConfigScrollViewer"
            };
            foreach (String name in dxrScrollViewerRegisteredNames)
            {
                DXRScrollViewer scrollViewer = FindName(name) as DXRScrollViewer;
                if (scrollViewer != null)
                {
                    DXRScrollViewer.ScrollCommandOrigin previousScrollOrigin = scrollViewer.scrollCommandOrigin;
                    scrollViewer.scrollCommandOrigin = DXRScrollViewer.ScrollCommandOrigin.SystemInitiated;
                    if (scrollViewer.shouldAutoScroll)
                    {
                        scrollViewer.ScrollToBottom();
                    }
                    scrollViewer.scrollCommandOrigin = previousScrollOrigin;
                }
                else
                {
                    postErrorMessage("A call to " + nameof(CreateMacroElementByApplicationMode) +
                        "was unable to acquire the DXRScrollViewer with registered name" + name
                        + ". Smart scrolling failed.");
                }
            }
            // Change the visibility of macro element separators based on mode count.
            if (macroElement.hasItemSeperator)
            {
                SetMacroElementSeparatorVisibilityByModeCount();
            }
            return macroElement;
        }
        private MacroElement CreateMacroElementByCurrentApplicationMode()
        {
            return CreateMacroElementByApplicationMode(CurrentApplicationMode);
        }
        private void DetermineQuickDescriptionVisibilityAndContentByApplicationMode(Mode mode)
        {
            TextBlock quickDescription = FindName("quickDescription") as TextBlock;
            if (quickDescription == null)
            {
                postErrorMessage("A call to " +
                    nameof(DetermineQuickDescriptionVisibilityAndContentByApplicationMode) +
                    " was unable to acquire the quick description text block.");
                return;
            }
            String description = GetModeDescriptorText(mode);
            if (GetModeCount(mode) == 0 &&
                description.Length > 0)
            {
                quickDescription.Text = description;
                quickDescription.Visibility = Visibility.Visible;
            }
            else
            {
                quickDescription.Text = "";
                quickDescription.Visibility = Visibility.Collapsed;
            }
        }
        readonly List<String> Master_MacroElementListeparatorNameList = new List<string>();
        public void CreateMacroElementSeparator(StackPanel verticalStackPanel)
        {
            try
            {
                Style separatorStyle_invisible = FindResource("separatortStyle_invisible") as Style;
                if (separatorStyle_invisible == null)
                {
                    postErrorMessage("Unable to aquire separator style.");
                    return;
                }
                Separator macroElementSeparator = new Separator()
                {
                    Style = separatorStyle_invisible
                };
                if (!WPFAssignAndRegisterName(macroElementSeparator,
                    nameof(macroElementSeparator) + macroElementSeparator.GetHashCode().ToString(),
                    Master_MacroElementListeparatorNameList))
                {
                    postErrorMessage("Unable to create macro element separator.");
                    return;
                }
                verticalStackPanel.Children.Add(macroElementSeparator);
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create a macro element separator: " +
                    ex.ToString());
            }
        }
        public void SetMacroElementSeparatorVisibilityByModeCount()
        {
            try
            {
                Style separatorStyle_visible = FindResource("separatortStyle_visible") as Style;
                if (separatorStyle_visible == null)
                {
                    postErrorMessage("A call to " + nameof(SetMacroElementSeparatorVisibilityByModeCount) +
                        " was unable to aquire the separator style (visible).");
                    return;
                }
                Style separatorStyle_invisible = FindResource("separatortStyle_invisible") as Style;
                if (separatorStyle_invisible == null)
                {
                    postErrorMessage("A call to " + nameof(SetMacroElementSeparatorVisibilityByModeCount) +
                        " was unable to aquire separator style (invisible).");
                    return;
                }
                foreach (String separatorID in Master_MacroElementListeparatorNameList)
                {
                    Separator separator = FindName(separatorID) as Separator;
                    if (separator == null)
                    {
                        postErrorMessage("A call to " + nameof(SetMacroElementSeparatorVisibilityByModeCount) +
                            " was unable to aquire separator with ID: " + separatorID);
                        return;
                    }
                    if (GetModeCount(CurrentApplicationMode) > 1)
                    {
                        separator.Style = separatorStyle_visible;
                    }
                    else
                    {
                        separator.Style = separatorStyle_invisible;
                    }
                }
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to set the visibility on a macro element separator: " +
                    ex.ToString());
            }
        }
        public String removeButtonToolTipPartialString = "Click to remove the associated ";
        public Grid GetRemoveButtonTrashCanIcon()
        {
            try
            {
                ImageSource trashCanImageSource = FindResource("trashCanImageResource") as ImageSource;
                if (trashCanImageSource == null)
                {
                    postErrorMessage("A call to " + nameof(GetRemoveButtonTrashCanIcon) +
                            " was unable to aquire the image source.");
                    return null;
                }
                Image trashCanImage = new Image()
                {
                    Source = trashCanImageSource,
                    Width = 30,
                    Height = 30,
                    Margin = new Thickness(0, 0, 0, 0)
                };
                Grid trashCanImageGrid = new Grid();
                trashCanImageGrid.Children.Add(trashCanImage);
                return trashCanImageGrid;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to retrieve a 'trash can' icon button: " +
                    ex.ToString());
                return null;
            }
        }
        public String CreateMacroElementButtonRow(Grid rootGrid, FrameworkElement buttonContainer)
        {
            try
            {
                if (rootGrid == null)
                {
                    postErrorMessage("A call to " + nameof(CreateMacroElementButtonRow) +
                        " was unable to acquire the root grid.");
                    return null;
                }
                if (buttonContainer == null)
                {
                    postErrorMessage("A call to " + nameof(CreateMacroElementButtonRow) +
                        " was unable to acquire the button target.");
                    return null;
                }
                buttonContainer.HorizontalAlignment = HorizontalAlignment.Right;
                Style separatorStyle_visible = FindResource("separatortStyle_visible") as Style;
                if (separatorStyle_visible == null)
                {
                    postErrorMessage("A call to " + nameof(CreateMacroElementButtonRow) +
                        " was unable to aquire the separator style (visible).");
                    return null;
                }
                RowDefinition buttonRow = new RowDefinition()
                {
                    Height = new GridLength(50)
                };
                if (!WPFAssignAndRegisterName(buttonRow, nameof(buttonRow) + buttonRow.GetHashCode().ToString()))
                {
                    postErrorMessage("A call to " + nameof(CreateMacroElementButtonRow) +
                        " was unable to register or crate a row definition.");
                    return null;
                }
                Separator separator = new Separator()
                {
                    MinHeight = 2.5,
                    MaxHeight = 10,
                    Margin = new Thickness(0, 0, 0, 0),
                    Padding = new Thickness(0, 0, 0, 0)
                };
                if (WPFAssignAndRegisterName(separator, nameof(separator) + separator.GetHashCode().ToString()))
                {
                    separator.Style = separatorStyle_visible;
                }
                else
                {
                    postErrorMessage("A call to " + nameof(CreateMacroElementButtonRow) +
                        " was unable to register or crate a separator.");
                    return null;
                }
                StackPanel sPanel = new StackPanel()
                {
                    Orientation = Orientation.Vertical,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Stretch,
                    Background = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#FF1A1A1A")),
                    //AllowDrop = true
                };
                if (WPFAssignAndRegisterName(sPanel, nameof(sPanel) + sPanel.GetHashCode().ToString()))
                {
                    sPanel.Children.Add(separator);
                    sPanel.Children.Add(buttonContainer);
                    // Drag and drop
                    //sPanel.Drop += new DragEventHandler(HandleJSONDragAndDrop);
                }
                else
                {
                    postErrorMessage("A call to " + nameof(CreateMacroElementButtonRow) +
                        " was unable to register or crate a Stack Panel.");
                    return null;
                }
                rootGrid.RowDefinitions.Add(buttonRow);
                sPanel.SetValue(Grid.RowProperty, rootGrid.RowDefinitions.Count - 1);
                rootGrid.Children.Add(sPanel);
                return sPanel.Name;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create a macro element button row: " +
                    ex.ToString());
                return null;
            }
        }
        public MacroElement CreateWelcomePane()
        {
            try
            {
                List<String> nameList = new List<string>();
                bool success = true;
                RowDefinition welcomeQuickTitleRow = new RowDefinition()
                {
                    Height = new GridLength(80)
                };
                if (!WPFAssignAndRegisterName(welcomeQuickTitleRow, nameof(welcomeQuickTitleRow), nameList))
                {
                    postErrorMessage("Unable to create " + nameof(welcomeQuickTitleRow));
                    success = false;
                }
                RowDefinition welcomeContentRow = new RowDefinition();
                if (!WPFAssignAndRegisterName(welcomeContentRow, nameof(welcomeContentRow), nameList))
                {
                    postErrorMessage("Unable to create " + nameof(welcomeContentRow));
                    success = false;
                }
                TextBlock welcomeQuickTitle = new TextBlock()
                {
                    Style = FindResource("quickTitleStyle") as Style,
                    VerticalAlignment = VerticalAlignment.Center
                };
                if (WPFAssignAndRegisterName(welcomeQuickTitle, nameof(welcomeQuickTitle), nameList))
                {
                    welcomeQuickTitle.SetValue(Grid.RowProperty, 0);
                    welcomeQuickTitle.SetValue(Grid.ColumnProperty, 1);
                    welcomeQuickTitle.Inlines.Add("RGA DXR state editor");
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(welcomeQuickTitle));
                    success = false;
                }
                // Welcome Pane Text Block
                TextBlock welcomePaneTextblock = new TextBlock()
                {
                    Foreground = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#FFFFFFFF")),
                    VerticalAlignment = VerticalAlignment.Top,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    MinWidth = 705,
                    Margin = new Thickness(20),
                    FontSize = 15
                };
                if (WPFAssignAndRegisterName(welcomePaneTextblock, nameof(welcomePaneTextblock), nameList))
                {
                    welcomePaneTextblock.SetValue(Grid.RowProperty, 1);
                    welcomePaneTextblock.SetValue(Grid.ColumnProperty, 1);
                    welcomePaneTextblock.Inlines.Add(welcomeText);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(welcomePaneTextblock));
                    success = false;
                }
                // Welcome Pane Grid
                Grid welcomePaneGrid = new Grid();
                if (WPFAssignAndRegisterName(welcomePaneGrid, nameof(welcomePaneGrid), nameList))
                {
                    welcomePaneGrid.RowDefinitions.Add(welcomeQuickTitleRow);
                    welcomePaneGrid.RowDefinitions.Add(welcomeContentRow);
                    welcomePaneGrid.Children.Add(welcomeQuickTitle);
                    welcomePaneGrid.Children.Add(welcomePaneTextblock);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(welcomePaneGrid));
                    success = false;
                }
                MacroElement macroElement;
                if (success)
                {
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.Welcome,
                        rootElement = welcomePaneGrid,
                        targetElement = FindName("viewGrid") as Panel,
                        regiteredChildrenNames = nameList,
                        hasItemSeperator = false
                    };
                }
                else
                {
                    postErrorMessage("Unable to create element for mode: " + nameof(Mode.Welcome));
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.ApplicationError,
                        rootElement = null,
                        targetElement = null,
                        regiteredChildrenNames = null
                    };
                }
                return macroElement;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create the welcome pane: " +
                    ex.ToString());
                return new MacroElement();
            }
        }
        public MacroElement CreateGeneralStateConfig()
        {
            try
            {
                //SetApplicationMode(Mode.GeneralStateConfig);
                List<String> nameList = new List<String>();
                bool success = true;
                RowDefinition quickTitleRow = new RowDefinition()
                {
                    Height = new GridLength(80)
                };
                if (!WPFAssignAndRegisterName(quickTitleRow, nameof(quickTitleRow), nameList))
                {
                    postErrorMessage("Unable to create " + nameof(quickTitleRow));
                    success = false;
                }
                RowDefinition contentRow = new RowDefinition();
                if (!WPFAssignAndRegisterName(contentRow, nameof(contentRow), nameList))
                {
                    postErrorMessage("Unable to create " + nameof(contentRow));
                    success = false;
                }
                TextBlock quickTitle = new TextBlock()
                {
                    Style = FindResource("quickTitleStyle") as Style
                };
                if (WPFAssignAndRegisterName(quickTitle, nameof(quickTitle), nameList))
                {
                    quickTitle.SetValue(Grid.RowProperty, 0);
                    quickTitle.SetValue(Grid.ColumnProperty, 1);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(quickTitle));
                    success = false;
                }
                TextBlock quickDescription = new TextBlock()
                {
                    Style = FindResource("quickDescriptionStyle") as Style
                };
                if (WPFAssignAndRegisterName(quickDescription, nameof(quickDescription), nameList))
                {
                    quickDescription.SetValue(Grid.RowProperty, 1);
                    quickDescription.SetValue(Grid.ColumnProperty, 1);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(quickDescription));
                    success = false;
                }
                // General State Config Button (Add <state>)
                Button addButton = new Button()
                {
                    Style = FindResource("primaryButtonStyle") as Style,
                    Margin = new Thickness(5, 5, 5, 5),
                    MinWidth = 135,
                    MinHeight = 35,
                    HorizontalAlignment = HorizontalAlignment.Right,
                    VerticalAlignment = VerticalAlignment.Center
                };
                if (WPFAssignAndRegisterName(addButton, nameof(addButton), nameList))
                {
                    addButton.SetValue(Grid.RowProperty, 2);
                    addButton.Click += new RoutedEventHandler(AddButton_Click);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(addButton));
                    success = false;
                }
                bool contentGridSuccess = true;
                Grid contentGrid = new Grid()
                {
                    VerticalAlignment = VerticalAlignment.Top,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    MinWidth = ContentViewSettings.ContentMinWidth,
                    Margin = new Thickness(0, 0, 0, 0)
                };
                if (WPFAssignAndRegisterName(contentGrid, nameof(contentGrid), nameList))
                {
                    contentGrid.SetValue(Grid.RowProperty, 0);
                }
                else
                {
                    contentGridSuccess = false;
                    postErrorMessage("Unable to create " + nameof(contentGrid));
                    success = false;
                }
                DXRScrollViewer generalStateConfigScrollViewer = new DXRScrollViewer()
                {
                    HorizontalScrollBarVisibility = ScrollBarVisibility.Auto,
                    VerticalScrollBarVisibility = ScrollBarVisibility.Auto,
                    VerticalContentAlignment = VerticalAlignment.Top,
                    HorizontalContentAlignment = HorizontalAlignment.Left,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top,
                    MinWidth = ContentViewSettings.ContentMinWidth,
                    Margin = new Thickness(0, 0, 5, 0)
                };
                if (WPFAssignAndRegisterName(generalStateConfigScrollViewer, nameof(generalStateConfigScrollViewer), nameList))
                {
                    generalStateConfigScrollViewer.SetValue(Grid.ColumnProperty, 1);
                    generalStateConfigScrollViewer.SetValue(Grid.RowProperty, 1);
                    if (contentGridSuccess)
                    {
                        generalStateConfigScrollViewer.Content = contentGrid;
                        generalStateConfigScrollViewer.ScrollChanged +=
                            new ScrollChangedEventHandler(HandleDXRScrollViewerScrollChanged);
                    }
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(generalStateConfigScrollViewer));
                    success = false;
                }
                //General State Config Grid
                Grid generalStateConfigGrid = new Grid()
                {
                    Margin = new Thickness(0, 0, 0, 0),
                    MinWidth = ContentViewSettings.ContentMinWidth,
                    HorizontalAlignment = HorizontalAlignment.Stretch
                };
                if (WPFAssignAndRegisterName(generalStateConfigGrid, nameof(generalStateConfigGrid), nameList))
                {
                    generalStateConfigGrid.SetValue(Grid.ColumnProperty, 1);
                    generalStateConfigGrid.RowDefinitions.Add(quickTitleRow);
                    generalStateConfigGrid.RowDefinitions.Add(contentRow);
                    generalStateConfigGrid.Children.Add(quickTitle);
                    generalStateConfigGrid.Children.Add(quickDescription);
                    generalStateConfigGrid.Children.Add(generalStateConfigScrollViewer);
                    // Add default button row.
                    CreateMacroElementButtonRow(generalStateConfigGrid, addButton);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(generalStateConfigGrid));
                    success = false;
                }
                MacroElement macroElement;
                if (success)
                {
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.GeneralStateConfig,
                        rootElement = generalStateConfigGrid,
                        targetElement = FindName("viewGrid") as Panel,
                        regiteredChildrenNames = nameList,
                        hasItemSeperator = false
                    };
                }
                else
                {
                    postErrorMessage("Unable to create element for mode: " + nameof(Mode.GeneralStateConfig));
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.ApplicationError,
                        rootElement = null,
                        targetElement = null,
                        regiteredChildrenNames = null
                    };
                }
                return macroElement;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create the general state configuration pane: " +
                    ex.ToString());
                return new MacroElement();
            }
        }
        public MacroElement CreateShaderConfig()
        {
            return CreateShaderConfig(0);
        }
        public MacroElement CreateShaderConfig(UInt32 nameModifier)
        {
            try
            {
                List<String> nameList = new List<String>();
                bool success = true;
                //
                // Type elements.
                //
                bool shouldAssociateshaderTypeCobmoBoxToJSON = true;
                ComboBox shaderTypeComboBox = new ComboBox()
                {
                    Margin = new Thickness(ContentViewSettings.ContentComboBoxMargin_left,
                        ContentViewSettings.ContentComboBoxMargin_top,
                        ContentViewSettings.ContentComboBoxMargin_right,
                        ContentViewSettings.ContentComboBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentComboBoxMinWidth
                };
                if (WPFAssignAndRegisterName(shaderTypeComboBox, nameof(shaderTypeComboBox) + nameModifier, nameList))
                {
                    shaderTypeComboBox.SetValue(Grid.RowProperty, 0);
                    shaderTypeComboBox.SetValue(Grid.ColumnProperty, 1);
                    shaderTypeComboBox.Style = FindResource("baseComboBoxStyle") as Style;
                    List<String> shaderTypeComboBoxItems =
                        new List<String>(new String[]
                        {
                            ConstStringBinaryDXILType,
                            ConstStringHLSLType
                        });

                    SetComboBoxItemContent(shaderTypeComboBox, shaderTypeComboBoxItems);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(shaderTypeComboBox) + nameModifier);
                    shouldAssociateshaderTypeCobmoBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label shaderTypeLabel = new Label()
                {
                    Content = "Type",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(shaderTypeComboBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(shaderTypeLabel, nameof(shaderTypeLabel) + nameModifier, nameList))
                {
                    shaderTypeLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(shaderTypeLabel) + nameModifier);
                    success = false;
                }
                StackPanel shaderTypeStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(shaderTypeStackPanel, nameof(shaderTypeStackPanel) + nameModifier, nameList))
                {
                    shaderTypeStackPanel.Children.Add(shaderTypeLabel);
                    shaderTypeStackPanel.Children.Add(shaderTypeComboBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(shaderTypeStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Entry point elements
                //
                bool shouldAssociateEntryPointTextBoxtoJSON = true;
                TextBox shaderEntryPointTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                };
                if (WPFAssignAndRegisterName(shaderEntryPointTextBox, nameof(shaderEntryPointTextBox) + nameModifier, nameList))
                {
                    shaderEntryPointTextBox.SetValue(Grid.RowProperty, 0);
                    shaderEntryPointTextBox.SetValue(Grid.ColumnProperty, 1);
                    shaderEntryPointTextBox.ToolTip = shaderEntryPointToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(shaderEntryPointTextBox) + nameModifier);
                    shouldAssociateEntryPointTextBoxtoJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label shaderEntryPointLabel = new Label()
                {
                    Content = "Entry point(s)",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(shaderEntryPointTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(shaderEntryPointLabel, nameof(shaderEntryPointLabel) + nameModifier, nameList))
                {
                    shaderEntryPointLabel.SetValue(Grid.ColumnProperty, 0);
                    shaderEntryPointLabel.ToolTip = shaderEntryPointToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(shaderEntryPointLabel) + nameModifier);
                    success = false;
                }
                StackPanel shaderEntryPointStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(shaderEntryPointStackPanel, nameof(shaderEntryPointStackPanel) + nameModifier, nameList))
                {
                    shaderEntryPointStackPanel.Children.Add(shaderEntryPointLabel);
                    shaderEntryPointStackPanel.Children.Add(shaderEntryPointTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(shaderEntryPointStackPanel) + nameModifier);
                    success = false;
                }
                //
                // File path elements.
                //
                bool shouldAssociateFilePathTextBoxtoJSON = true;
                bool shouldConnectShaderFilePathBrowseButton = true;
                Button shaderFilePathBrowseButton = new Button()
                {
                    Style = FindResource("browseButtonStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentBrowseButtonMargin_left,
                        ContentViewSettings.ContentBrowseButtonMargin_top,
                        ContentViewSettings.ContentBrowseButtonMargin_right,
                        ContentViewSettings.ContentBrowseButtonMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    Width = ContentViewSettings.ContentBrowseButtonWidth
                };
                if (!WPFAssignAndRegisterName(shaderFilePathBrowseButton, nameof(shaderFilePathBrowseButton) + nameModifier, nameList))
                {
                    shouldConnectShaderFilePathBrowseButton = false;
                    postErrorMessage("Unable to create " + nameof(shaderFilePathBrowseButton) + nameModifier);
                    success = false;
                }
                TextBox shaderFilePathTextBox = new TextBox()
                {
                    Style = FindResource("defaultYetInvalidTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                };
                if (WPFAssignAndRegisterName(shaderFilePathTextBox, nameof(shaderFilePathTextBox) + nameModifier, nameList))
                {
                    if (shouldConnectShaderFilePathBrowseButton)
                    {
                        shaderFilePathBrowseButton.Click += new RoutedEventHandler(BrowseForFilePath);
                        shaderFilePathBrowseButton.Tag = shaderFilePathTextBox.Name;
                    }
                    shaderFilePathTextBox.SetValue(Grid.ColumnProperty, 1);
                    shaderFilePathTextBox.ToolTip = shaderFilePathToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(shaderFilePathTextBox) + nameModifier);
                    success = false;
                    shouldAssociateFilePathTextBoxtoJSON = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label shaderFilePathLabel = new Label()
                {
                    Content = "File path",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(shaderFilePathTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(shaderFilePathLabel, nameof(shaderFilePathLabel) + nameModifier, nameList))
                {
                    shaderFilePathLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(shaderFilePathLabel) + nameModifier);
                    success = false;
                }
                StackPanel shaderFilePathStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(shaderFilePathStackPanel, nameof(shaderFilePathStackPanel) + nameModifier, nameList))
                {
                    shaderFilePathStackPanel.Children.Add(shaderFilePathLabel);
                    shaderFilePathStackPanel.Children.Add(shaderFilePathTextBox);
                    shaderFilePathStackPanel.Children.Add(shaderFilePathBrowseButton);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(shaderFilePathStackPanel) + nameModifier);
                    success = false;
                }
                StackPanel shaderVerticalStackPanel = new StackPanel()
                {
                    HorizontalAlignment = HorizontalAlignment.Stretch
                };
                if (WPFAssignAndRegisterName(shaderVerticalStackPanel, nameof(shaderVerticalStackPanel) + nameModifier, nameList))
                {
                    // Add separator.
                    CreateMacroElementSeparator(shaderVerticalStackPanel);
                    shaderVerticalStackPanel.SetValue(Grid.ColumnProperty, 0);
                    shaderVerticalStackPanel.Children.Add(shaderTypeStackPanel);
                    shaderVerticalStackPanel.Children.Add(shaderFilePathStackPanel);
                    shaderVerticalStackPanel.Children.Add(shaderEntryPointStackPanel);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(shaderVerticalStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Top level elements.
                //
                bool shouldConnectButtonEvent = true;
                Button shaderRemoveItemButton = new Button()
                {
                    Style = FindResource("removeItemButtonStyle") as Style,
                    Content = GetRemoveButtonTrashCanIcon(),
                    Margin = new Thickness(ContentViewSettings.ContentRemoveButton_withoutBrowse_withTextBox_MarginLeft, 0, 0,
                    ContentViewSettings.ContentRemoveButton_withoutBrowse_withTextBox_MarginBottom)
                };
                if (WPFAssignAndRegisterName(shaderRemoveItemButton, nameof(shaderRemoveItemButton) + nameModifier, nameList))
                {
                    shaderRemoveItemButton.SetValue(Grid.ColumnProperty, 1);
                    shaderRemoveItemButton.ToolTip = removeButtonToolTipPartialString + GetModeAsText(Mode.Shader);
                }
                else
                {
                    shouldConnectButtonEvent = false;
                    postErrorMessage("Unable to create " + nameof(shaderRemoveItemButton) + nameModifier);
                    success = false;
                }
                ColumnDefinition shaderLabelAndTextboxCol = new ColumnDefinition()
                {
                    Width = new GridLength(1.0, GridUnitType.Auto),
                    MinWidth = 100
                };
                if (!WPFAssignAndRegisterName(shaderLabelAndTextboxCol, nameof(shaderLabelAndTextboxCol) + nameModifier, nameList))
                {
                    postErrorMessage("Unable to create " + nameof(shaderLabelAndTextboxCol) + nameModifier);
                    success = false;
                }
                Grid shaderModeRootElement = new Grid()
                {
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top,
                };
                if (WPFAssignAndRegisterName(shaderModeRootElement, nameof(shaderModeRootElement) + nameModifier, nameList))
                {
                    String macroElementID = nameof(shaderModeRootElement) + nameModifier;
                    shaderModeRootElement.ColumnDefinitions.Add(shaderLabelAndTextboxCol);
                    shaderModeRootElement.Children.Add(shaderVerticalStackPanel);
                    shaderModeRootElement.Tag = macroElementID;
                    // Associate button mechanics.
                    if (shouldConnectButtonEvent)
                    {
                        // Tuple layout: <parentName, grandParentName>
                        shaderRemoveItemButton.Tag = macroElementID;
                        shaderRemoveItemButton.Click += new RoutedEventHandler(HandleTrashcanIconClick);
                        shaderTypeStackPanel.Children.Add(shaderRemoveItemButton);
                    }
                    // Associate JSON mechanics
                    if (shouldAssociateEntryPointTextBoxtoJSON)
                    {
                        shaderEntryPointTextBox.Tag = macroElementID;
                        shaderEntryPointTextBox.TextChanged +=
                            new TextChangedEventHandler(AddShaderEntryPointByID);
                    }
                    if (shouldAssociateFilePathTextBoxtoJSON)
                    {
                        shaderFilePathTextBox.Tag = macroElementID;
                        shaderFilePathTextBox.Text = defaultYetInvalidFilePathString;
                        shaderFilePathTextBox.TextChanged +=
                            new TextChangedEventHandler(AddShaderFilePathByID);
                        shaderFilePathTextBox.GotFocus += ClearDefaultFilePathText;
                    }
                    if (shouldAssociateshaderTypeCobmoBoxToJSON)
                    {
                        shaderTypeComboBox.Tag = macroElementID;
                        shaderTypeComboBox.SelectionChanged +=
                            new SelectionChangedEventHandler(AddShaderTypeByID);
                        Mode previousApplicationMode = CurrentApplicationMode;
                        SetApplicationMode(Mode.Shader);
                        shaderTypeComboBox.SelectedValue = shaderTypeComboBox.Items[0];
                        SetApplicationMode(previousApplicationMode);
                    }
                }
                else
                {
                    shouldConnectButtonEvent = false;
                    postErrorMessage("Unable to create " + nameof(shaderModeRootElement) + nameModifier);
                    success = false;
                }
                MacroElement macroElement;
                if (success)
                {
                    ModeCount.ShaderCount++;
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.Shader,
                        rootElement = shaderModeRootElement,
                        targetElement = FindName("contentGrid") as Panel,
                        regiteredChildrenNames = nameList,
                        hasItemSeperator = true
                    };
                }
                else
                {
                    postErrorMessage("Unable to create element for mode: " + nameof(Mode.Shader));
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.ApplicationError,
                        rootElement = null,
                        targetElement = null,
                        regiteredChildrenNames = null
                    };
                }
                return macroElement;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create the shader configuration pane: " +
                    ex.ToString());
                return new MacroElement();
            }
        }
        public MacroElement CreateHitGroupConfig()
        {
            return CreateHitGroupConfig(0);
        }
        public MacroElement CreateHitGroupConfig(UInt32 nameModifier)
        {
            try
            {
                List<String> nameList = new List<string>();
                bool success = true;
                //
                // Type elements.
                //
                bool shouldAssociateTypeComboBoxToJSON = true;
                ComboBox hitGroupTypeComboBox = new ComboBox()
                {
                    Margin = new Thickness(ContentViewSettings.ContentComboBoxMargin_left,
                        ContentViewSettings.ContentComboBoxMargin_top,
                        ContentViewSettings.ContentComboBoxMargin_right,
                        ContentViewSettings.ContentComboBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentComboBoxMinWidth
                };
                if (WPFAssignAndRegisterName(hitGroupTypeComboBox, nameof(hitGroupTypeComboBox) + nameModifier, nameList))
                {
                    hitGroupTypeComboBox.SetValue(Grid.RowProperty, 0);
                    hitGroupTypeComboBox.SetValue(Grid.ColumnProperty, 1);
                    hitGroupTypeComboBox.Style = FindResource("baseComboBoxStyle") as Style;
                    List<String> hitGroupTypeComboBoxItems =
                        new List<String>(new String[]
                        {
                            "D3D12_HIT_GROUP_TYPE_TRIANGLES"
                        });

                    SetComboBoxItemContent(hitGroupTypeComboBox, hitGroupTypeComboBoxItems);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupTypeComboBox) + nameModifier);
                    shouldAssociateTypeComboBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label hitGroupTypeLabel = new Label()
                {
                    Content = "Type",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(hitGroupTypeComboBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(hitGroupTypeLabel, nameof(hitGroupTypeLabel) + nameModifier, nameList))
                {
                    hitGroupTypeLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupTypeLabel) + nameModifier);
                    success = false;
                }
                StackPanel hitGroupTypeStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(hitGroupTypeStackPanel, nameof(hitGroupTypeStackPanel) + nameModifier, nameList))
                {
                    hitGroupTypeStackPanel.Children.Add(hitGroupTypeLabel);
                    hitGroupTypeStackPanel.Children.Add(hitGroupTypeComboBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupTypeStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Name elements.
                //
                bool shouldAssociateNameTextBoxToJSON = true;
                TextBox hitGroupNameTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                };
                if (WPFAssignAndRegisterName(hitGroupNameTextBox, nameof(hitGroupNameTextBox) + nameModifier, nameList))
                {
                    hitGroupNameTextBox.SetValue(Grid.RowProperty, 0);
                    hitGroupNameTextBox.SetValue(Grid.ColumnProperty, 1);
                    hitGroupNameTextBox.ToolTip = hitGroupNameToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupNameTextBox) + nameModifier);
                    shouldAssociateNameTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label hitGroupNameLabel = new Label()
                {
                    Content = "Name",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(hitGroupNameTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(hitGroupNameLabel, nameof(hitGroupNameLabel) + nameModifier, nameList))
                {
                    hitGroupNameLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupNameLabel) + nameModifier);
                    success = false;
                }
                StackPanel hitGroupNameStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(hitGroupNameStackPanel, nameof(hitGroupNameStackPanel) + nameModifier, nameList))
                {
                    hitGroupNameStackPanel.Children.Add(hitGroupNameLabel);
                    hitGroupNameStackPanel.Children.Add(hitGroupNameTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupNameStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Intersection shader elements.
                //
                bool shouldAssociateIntersectionShaderTextBoxToJSON = true;
                TextBox hitGroupIntersectionShaderTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                };
                if (WPFAssignAndRegisterName(hitGroupIntersectionShaderTextBox, nameof(hitGroupIntersectionShaderTextBox) + nameModifier, nameList))
                {
                    hitGroupIntersectionShaderTextBox.SetValue(Grid.RowProperty, 0);
                    hitGroupIntersectionShaderTextBox.SetValue(Grid.ColumnProperty, 1);
                    hitGroupIntersectionShaderTextBox.ToolTip = hitGroupIntersectionShaderToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupIntersectionShaderTextBox) + nameModifier);
                    success = false;
                    shouldAssociateIntersectionShaderTextBoxToJSON = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label hitGroupIntersectionShaderLabel = new Label()
                {
                    Content = "Intersection shader",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(hitGroupIntersectionShaderTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(hitGroupIntersectionShaderLabel, nameof(hitGroupIntersectionShaderLabel) + nameModifier, nameList))
                {
                    hitGroupIntersectionShaderLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupIntersectionShaderLabel) + nameModifier);
                    success = false;
                }
                StackPanel hitGroupIntersectionShaderStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(hitGroupIntersectionShaderStackPanel, nameof(hitGroupIntersectionShaderStackPanel) + nameModifier, nameList))
                {
                    hitGroupIntersectionShaderStackPanel.Children.Add(hitGroupIntersectionShaderLabel);
                    hitGroupIntersectionShaderStackPanel.Children.Add(hitGroupIntersectionShaderTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupIntersectionShaderStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Any hit shader elements.
                //
                bool shouldAssociateAnyHitShaderTextBoxToJSON = true;
                TextBox hitGroupAnyHitShaderTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                };
                if (WPFAssignAndRegisterName(hitGroupAnyHitShaderTextBox, nameof(hitGroupAnyHitShaderTextBox) + nameModifier, nameList))
                {
                    hitGroupAnyHitShaderTextBox.SetValue(Grid.RowProperty, 0);
                    hitGroupAnyHitShaderTextBox.SetValue(Grid.ColumnProperty, 1);
                    hitGroupAnyHitShaderTextBox.ToolTip = hitGroupAnyHitShaderToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupAnyHitShaderTextBox) + nameModifier);
                    success = false;
                    shouldAssociateAnyHitShaderTextBoxToJSON = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label hitGroupAnyHitShaderLabel = new Label()
                {
                    Content = "Any hit shader",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(hitGroupAnyHitShaderTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(hitGroupAnyHitShaderLabel, nameof(hitGroupAnyHitShaderLabel) + nameModifier, nameList))
                {
                    hitGroupAnyHitShaderLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupAnyHitShaderLabel) + nameModifier);
                    success = false;
                }
                StackPanel hitGroupAnyHitShaderStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(hitGroupAnyHitShaderStackPanel, nameof(hitGroupAnyHitShaderStackPanel) + nameModifier, nameList))
                {
                    hitGroupAnyHitShaderStackPanel.Children.Add(hitGroupAnyHitShaderLabel);
                    hitGroupAnyHitShaderStackPanel.Children.Add(hitGroupAnyHitShaderTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupAnyHitShaderStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Closest hit shader elements.
                //
                bool shouldAssociateClosestHitShaderTextBoxToJSON = true;
                TextBox hitGroupClosestHitShaderTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                };
                if (WPFAssignAndRegisterName(hitGroupClosestHitShaderTextBox, nameof(hitGroupClosestHitShaderTextBox) + nameModifier, nameList))
                {
                    hitGroupClosestHitShaderTextBox.SetValue(Grid.RowProperty, 0);
                    hitGroupClosestHitShaderTextBox.SetValue(Grid.ColumnProperty, 1);
                    hitGroupClosestHitShaderTextBox.ToolTip = hitGroupClosestHitShaderToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupClosestHitShaderTextBox) + nameModifier);
                    shouldAssociateClosestHitShaderTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label hitGroupClosestHitShaderLabel = new Label()
                {
                    Content = "Closest hit shader",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(hitGroupClosestHitShaderTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(hitGroupClosestHitShaderLabel, nameof(hitGroupClosestHitShaderLabel) + nameModifier, nameList))
                {
                    hitGroupClosestHitShaderLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupClosestHitShaderLabel) + nameModifier);
                    success = false;
                }
                StackPanel hitGroupClosestHitShaderStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(hitGroupClosestHitShaderStackPanel, nameof(hitGroupClosestHitShaderStackPanel) + nameModifier, nameList))
                {
                    hitGroupClosestHitShaderStackPanel.Children.Add(hitGroupClosestHitShaderLabel);
                    hitGroupClosestHitShaderStackPanel.Children.Add(hitGroupClosestHitShaderTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupClosestHitShaderStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Vertical Stack Panel
                //
                StackPanel hitGroupVerticalStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Vertical,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top,
                    MinWidth = ContentViewSettings.ContentMinWidth
                };
                if (WPFAssignAndRegisterName(hitGroupVerticalStackPanel, nameof(hitGroupVerticalStackPanel) + nameModifier, nameList))
                {
                    // Add separator.
                    CreateMacroElementSeparator(hitGroupVerticalStackPanel);
                    hitGroupVerticalStackPanel.Children.Add(hitGroupTypeStackPanel);
                    hitGroupVerticalStackPanel.Children.Add(hitGroupNameStackPanel);
                    hitGroupVerticalStackPanel.Children.Add(hitGroupIntersectionShaderStackPanel);
                    hitGroupVerticalStackPanel.Children.Add(hitGroupAnyHitShaderStackPanel);
                    hitGroupVerticalStackPanel.Children.Add(hitGroupClosestHitShaderStackPanel);
                    hitGroupVerticalStackPanel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupVerticalStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Top level elements.
                //
                bool shouldConnectButtonEvent = true;
                Button hitGroupRemoveItemButton = new Button()
                {
                    Style = FindResource("removeItemButtonStyle") as Style,
                    Content = GetRemoveButtonTrashCanIcon(),
                    Margin = new Thickness(ContentViewSettings.ContentRemoveButton_withoutBrowse_withTextBox_MarginLeft, 0, 0,
                    ContentViewSettings.ContentRemoveButton_withoutBrowse_withTextBox_MarginBottom)
                };
                if (WPFAssignAndRegisterName(hitGroupRemoveItemButton, nameof(hitGroupRemoveItemButton) + nameModifier, nameList))
                {
                    hitGroupRemoveItemButton.SetValue(Grid.ColumnProperty, 1);
                    hitGroupRemoveItemButton.ToolTip = removeButtonToolTipPartialString + GetModeAsText(Mode.HitGroup);
                }
                else
                {
                    shouldConnectButtonEvent = false;
                    postErrorMessage("Unable to create " + nameof(hitGroupRemoveItemButton) + nameModifier);
                    success = false;
                }
                ColumnDefinition hitGroupLabelAndTextboxCol = new ColumnDefinition()
                {
                    Width = new GridLength(1.0, GridUnitType.Auto),
                    MinWidth = 100
                };
                if (!WPFAssignAndRegisterName(hitGroupLabelAndTextboxCol, nameof(hitGroupLabelAndTextboxCol) + nameModifier, nameList))
                {
                    postErrorMessage("Unable to create " + nameof(hitGroupLabelAndTextboxCol) + nameModifier);
                    success = false;
                }
                Grid hitGroupModeRootElement = new Grid()
                {
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top,
                };
                if (WPFAssignAndRegisterName(hitGroupModeRootElement, nameof(hitGroupModeRootElement) + nameModifier, nameList))
                {
                    String macroElementID = nameof(hitGroupModeRootElement) + nameModifier;
                    hitGroupModeRootElement.ColumnDefinitions.Add(hitGroupLabelAndTextboxCol);
                    hitGroupModeRootElement.Children.Add(hitGroupVerticalStackPanel);
                    hitGroupModeRootElement.Tag = macroElementID;
                    // Associate button mechanics.
                    if (shouldConnectButtonEvent)
                    {
                        // Tuple layout: <parentName, grandParentName>
                        hitGroupRemoveItemButton.Tag = macroElementID;
                        hitGroupRemoveItemButton.Click += new RoutedEventHandler(HandleTrashcanIconClick);
                        hitGroupTypeStackPanel.Children.Add(hitGroupRemoveItemButton);
                    }
                    // Associate JSON mechanics.
                    if (shouldAssociateTypeComboBoxToJSON)
                    {
                        hitGroupTypeComboBox.Tag = macroElementID;
                        hitGroupTypeComboBox.SelectionChanged += new SelectionChangedEventHandler(AddHitGroupTypeByID);
                        Mode previousApplicationMode = CurrentApplicationMode;
                        SetApplicationMode(Mode.HitGroup);
                        hitGroupTypeComboBox.SelectedValue = hitGroupTypeComboBox.Items[0];
                        SetApplicationMode(previousApplicationMode);
                    }
                    if (shouldAssociateNameTextBoxToJSON)
                    {
                        hitGroupNameTextBox.Tag = macroElementID;
                        hitGroupNameTextBox.TextChanged +=
                            new TextChangedEventHandler(AddHitGroupNameByID);
                        hitGroupNameTextBox.Text = "HitGroup" + nameModifier;
                    }
                    if (shouldAssociateIntersectionShaderTextBoxToJSON)
                    {
                        hitGroupIntersectionShaderTextBox.Tag = macroElementID;
                        hitGroupIntersectionShaderTextBox.TextChanged += new TextChangedEventHandler(AddHitGroupIntersectionShaderByID);
                    }
                    if (shouldAssociateAnyHitShaderTextBoxToJSON)
                    {
                        hitGroupAnyHitShaderTextBox.Tag = macroElementID;
                        hitGroupAnyHitShaderTextBox.TextChanged += new TextChangedEventHandler(AddHitGroupAnyHitShaderByID);
                    }
                    if (shouldAssociateClosestHitShaderTextBoxToJSON)
                    {
                        hitGroupClosestHitShaderTextBox.Tag = macroElementID;
                        hitGroupClosestHitShaderTextBox.TextChanged += new TextChangedEventHandler(AddHitGroupClosestHitShaderByID);
                    }
                }
                else
                {
                    shouldConnectButtonEvent = false;
                    postErrorMessage("Unable to create " + nameof(hitGroupModeRootElement) + nameModifier);
                    success = false;
                }
                MacroElement macroElement;
                if (success)
                {
                    ModeCount.HitGroupCount++;
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.HitGroup,
                        rootElement = hitGroupModeRootElement,
                        targetElement = FindName("contentGrid") as Panel,
                        regiteredChildrenNames = nameList,
                        hasItemSeperator = true
                    };
                }
                else
                {
                    postErrorMessage("Unable to create element for mode: " + nameof(Mode.HitGroup));
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.ApplicationError,
                        rootElement = null,
                        targetElement = null,
                        regiteredChildrenNames = null
                    };
                }
                return macroElement;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create the hit group configuration pane: " +
                    ex.ToString());
                return new MacroElement();
            }
        }
        public MacroElement CreateLocalRootSignatureConfig()
        {
            return CreateLocalRootSignatureConfig(0);
        }
        public MacroElement CreateLocalRootSignatureConfig(UInt32 nameModifier)
        {
            try
            {
                List<String> nameList = new List<string>();
                bool success = true;
                //
                // Type elements.
                //
                bool shouldAssociateLocalRootSignatureTypeCobmoBoxToJSON = true;
                ComboBox localRootSignaturesTypeComboBox = new ComboBox()
                {
                    Margin = new Thickness(ContentViewSettings.ContentComboBoxMargin_left,
                        ContentViewSettings.ContentComboBoxMargin_top,
                        ContentViewSettings.ContentComboBoxMargin_right,
                        ContentViewSettings.ContentComboBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentComboBoxMinWidth
                };
                if (WPFAssignAndRegisterName(localRootSignaturesTypeComboBox, nameof(localRootSignaturesTypeComboBox) + nameModifier, nameList))
                {
                    localRootSignaturesTypeComboBox.SetValue(Grid.RowProperty, 0);
                    localRootSignaturesTypeComboBox.SetValue(Grid.ColumnProperty, 1);
                    localRootSignaturesTypeComboBox.Style = FindResource("baseComboBoxStyle") as Style;
                    List<String> localRootSignatureTypeComboBoxItems =
                        new List<String>(new String[]
                        {
                            ConstStringBinaryDXILType
                        });

                    SetComboBoxItemContent(localRootSignaturesTypeComboBox, localRootSignatureTypeComboBoxItems);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesTypeComboBox) + nameModifier);
                    shouldAssociateLocalRootSignatureTypeCobmoBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label localRootSignaturesTypeLabel = new Label()
                {
                    Content = "Type",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(localRootSignaturesTypeComboBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(localRootSignaturesTypeLabel, nameof(localRootSignaturesTypeLabel) + nameModifier, nameList))
                {
                    localRootSignaturesTypeLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesTypeLabel) + nameModifier);
                    success = false;
                }
                StackPanel localRootSignaturesTypeStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(localRootSignaturesTypeStackPanel, nameof(localRootSignaturesTypeStackPanel) + nameModifier, nameList))
                {
                    localRootSignaturesTypeStackPanel.Children.Add(localRootSignaturesTypeLabel);
                    localRootSignaturesTypeStackPanel.Children.Add(localRootSignaturesTypeComboBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesTypeStackPanel) + nameModifier);
                    success = false;
                }
                //
                // File path elements.
                //
                bool shouldConnectlocalRootSignaturesFilePathBrowseButton = true;
                Button localRootSignaturesFilePathBrowseButton = new Button()
                {
                    Style = FindResource("browseButtonStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentBrowseButtonMargin_left,
                        ContentViewSettings.ContentBrowseButtonMargin_top,
                        ContentViewSettings.ContentBrowseButtonMargin_right,
                        ContentViewSettings.ContentBrowseButtonMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    Width = ContentViewSettings.ContentBrowseButtonWidth
                };
                if (!WPFAssignAndRegisterName(localRootSignaturesFilePathBrowseButton, nameof(localRootSignaturesFilePathBrowseButton) + nameModifier, nameList))
                {
                    shouldConnectlocalRootSignaturesFilePathBrowseButton = false;
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesFilePathBrowseButton) + nameModifier);
                    success = false;
                }
                bool shouldAssociateLocalRootSignatureFilepathTextBoxToJSON = true;
                TextBox localRootSignaturesFilePathTextBox = new TextBox()
                {
                    Style = FindResource("defaultYetInvalidTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                };
                if (WPFAssignAndRegisterName(localRootSignaturesFilePathTextBox, nameof(localRootSignaturesFilePathTextBox) + nameModifier, nameList))
                {
                    if (shouldConnectlocalRootSignaturesFilePathBrowseButton)
                    {
                        localRootSignaturesFilePathBrowseButton.Click += new RoutedEventHandler(BrowseForFilePath);
                        localRootSignaturesFilePathBrowseButton.Tag = localRootSignaturesFilePathTextBox.Name;
                    }
                    localRootSignaturesFilePathTextBox.SetValue(Grid.ColumnProperty, 1);
                    localRootSignaturesFilePathTextBox.ToolTip = localRootSignaturesFilePathToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesFilePathTextBox) + nameModifier);
                    shouldAssociateLocalRootSignatureFilepathTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label localRootSignaturesFilePathLabel = new Label()
                {
                    Content = "File path",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(localRootSignaturesFilePathTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(localRootSignaturesFilePathLabel, nameof(localRootSignaturesFilePathLabel) + nameModifier, nameList))
                {
                    localRootSignaturesFilePathLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesFilePathLabel) + nameModifier);
                    success = false;
                }
                StackPanel localRootSignaturesFilePathStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(localRootSignaturesFilePathStackPanel, nameof(localRootSignaturesFilePathStackPanel) + nameModifier, nameList))
                {
                    localRootSignaturesFilePathStackPanel.Children.Add(localRootSignaturesFilePathLabel);
                    localRootSignaturesFilePathStackPanel.Children.Add(localRootSignaturesFilePathTextBox);
                    localRootSignaturesFilePathStackPanel.Children.Add(localRootSignaturesFilePathBrowseButton);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesFilePathStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Macro Name shader elements.
                //
                bool shouldAssociateLocalRootSignatureMacroNameShaderTextBoxToJSON = true;
                TextBox localRootSignaturesMacroNameShaderTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                };
                if (WPFAssignAndRegisterName(localRootSignaturesMacroNameShaderTextBox, nameof(localRootSignaturesMacroNameShaderTextBox) + nameModifier, nameList))
                {
                    localRootSignaturesMacroNameShaderTextBox.SetValue(Grid.RowProperty, 0);
                    localRootSignaturesMacroNameShaderTextBox.SetValue(Grid.ColumnProperty, 1);
                    localRootSignaturesMacroNameShaderTextBox.ToolTip = localRootSignaturesMacroNameShaderToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesMacroNameShaderTextBox) + nameModifier);
                    shouldAssociateLocalRootSignatureMacroNameShaderTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label localRootSignaturesMacroNameShaderLabel = new Label()
                {
                    Content = "Macro name (HLSL)",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(localRootSignaturesMacroNameShaderTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(localRootSignaturesMacroNameShaderLabel, nameof(localRootSignaturesMacroNameShaderLabel) + nameModifier, nameList))
                {
                    localRootSignaturesMacroNameShaderLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesMacroNameShaderLabel) + nameModifier);
                    success = false;
                }
                StackPanel localRootSignaturesMacroNameShaderStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top,
                    // Note: Needs to be visible/settable in the JSON representation, but not
                    //       editable by the user for the time being.
                    Visibility = Visibility.Collapsed
                };
                if (WPFAssignAndRegisterName(localRootSignaturesMacroNameShaderStackPanel, nameof(localRootSignaturesMacroNameShaderStackPanel) + nameModifier, nameList))
                {
                    localRootSignaturesMacroNameShaderStackPanel.Children.Add(localRootSignaturesMacroNameShaderLabel);
                    localRootSignaturesMacroNameShaderStackPanel.Children.Add(localRootSignaturesMacroNameShaderTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesMacroNameShaderStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Exports elements.
                //
                bool shouldAssociateLocalRootSignatureExportsTextBoxToJSON = true;
                TextBox localRootSignaturesExportsTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                };
                if (WPFAssignAndRegisterName(localRootSignaturesExportsTextBox, nameof(localRootSignaturesExportsTextBox) + nameModifier, nameList))
                {
                    localRootSignaturesExportsTextBox.SetValue(Grid.RowProperty, 0);
                    localRootSignaturesExportsTextBox.SetValue(Grid.ColumnProperty, 1);
                    localRootSignaturesExportsTextBox.ToolTip = localRootSignaturesExportsToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesExportsTextBox) + nameModifier);
                    shouldAssociateLocalRootSignatureExportsTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label localRootSignaturesExportsLabel = new Label()
                {
                    Content = "Exports",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(localRootSignaturesExportsTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(localRootSignaturesExportsLabel, nameof(localRootSignaturesExportsLabel) + nameModifier, nameList))
                {
                    localRootSignaturesExportsLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesExportsLabel) + nameModifier);
                    success = false;
                }
                StackPanel localRootSignaturesExportsStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(localRootSignaturesExportsStackPanel, nameof(localRootSignaturesExportsStackPanel) + nameModifier, nameList))
                {
                    localRootSignaturesExportsStackPanel.Children.Add(localRootSignaturesExportsLabel);
                    localRootSignaturesExportsStackPanel.Children.Add(localRootSignaturesExportsTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesExportsStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Vertical Stack Panel
                //
                StackPanel localRootSignaturesVerticalStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Vertical,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top,
                    MinWidth = ContentViewSettings.ContentMinWidth
                };
                if (WPFAssignAndRegisterName(localRootSignaturesVerticalStackPanel, nameof(localRootSignaturesVerticalStackPanel) + nameModifier, nameList))
                {
                    // Add separator.
                    CreateMacroElementSeparator(localRootSignaturesVerticalStackPanel);
                    localRootSignaturesVerticalStackPanel.Children.Add(localRootSignaturesTypeStackPanel);
                    localRootSignaturesVerticalStackPanel.Children.Add(localRootSignaturesFilePathStackPanel);
                    localRootSignaturesVerticalStackPanel.Children.Add(localRootSignaturesMacroNameShaderStackPanel);
                    localRootSignaturesVerticalStackPanel.Children.Add(localRootSignaturesExportsStackPanel);
                    localRootSignaturesVerticalStackPanel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesVerticalStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Top level elements.
                //
                bool shouldConnectButtonEvent = true;
                Button localRootSignaturesRemoveItemButton = new Button()
                {
                    Style = FindResource("removeItemButtonStyle") as Style,
                    Content = GetRemoveButtonTrashCanIcon(),
                    Margin = new Thickness(ContentViewSettings.ContentRemoveButton_withoutBrowse_withTextBox_MarginLeft, 0, 0,
                    ContentViewSettings.ContentRemoveButton_withoutBrowse_withTextBox_MarginBottom)
                };
                if (WPFAssignAndRegisterName(localRootSignaturesRemoveItemButton, nameof(localRootSignaturesRemoveItemButton) + nameModifier, nameList))
                {
                    localRootSignaturesRemoveItemButton.SetValue(Grid.ColumnProperty, 1);
                    localRootSignaturesRemoveItemButton.ToolTip = removeButtonToolTipPartialString + GetModeAsText(Mode.LocalRootSignature);
                }
                else
                {
                    shouldConnectButtonEvent = false;
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesRemoveItemButton) + nameModifier);
                    success = false;
                }
                ColumnDefinition localRootSignaturesLabelAndTextboxCol = new ColumnDefinition()
                {
                    Width = new GridLength(1.0, GridUnitType.Auto),
                    MinWidth = 100
                };
                if (!WPFAssignAndRegisterName(localRootSignaturesLabelAndTextboxCol, nameof(localRootSignaturesLabelAndTextboxCol) + nameModifier, nameList))
                {
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesLabelAndTextboxCol) + nameModifier);
                    success = false;
                }
                Grid localRootSignaturesModeRootElement = new Grid()
                {
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(localRootSignaturesModeRootElement, nameof(localRootSignaturesModeRootElement) + nameModifier, nameList))
                {
                    String macroElementID = nameof(localRootSignaturesModeRootElement) + nameModifier;
                    localRootSignaturesModeRootElement.ColumnDefinitions.Add(localRootSignaturesLabelAndTextboxCol);
                    localRootSignaturesModeRootElement.Children.Add(localRootSignaturesVerticalStackPanel);
                    localRootSignaturesModeRootElement.Tag = macroElementID;
                    // Associate button mechanics.
                    if (shouldConnectButtonEvent)
                    {
                        // Tuple layout: <parentName, grandParentName>
                        localRootSignaturesRemoveItemButton.Tag = macroElementID;
                        localRootSignaturesRemoveItemButton.Click += new RoutedEventHandler(HandleTrashcanIconClick);
                        localRootSignaturesTypeStackPanel.Children.Add(localRootSignaturesRemoveItemButton);
                    }
                    // Associate JSON mechanics.
                    if (shouldAssociateLocalRootSignatureFilepathTextBoxToJSON)
                    {
                        localRootSignaturesFilePathTextBox.Tag = macroElementID;
                        localRootSignaturesFilePathTextBox.Text = defaultYetInvalidFilePathString;
                        localRootSignaturesFilePathTextBox.TextChanged +=
                            new TextChangedEventHandler(AddLocalRootSignatureFilePathByID);
                        localRootSignaturesFilePathTextBox.GotFocus += ClearDefaultFilePathText;
                    }
                    if (shouldAssociateLocalRootSignatureMacroNameShaderTextBoxToJSON)
                    {
                        localRootSignaturesMacroNameShaderTextBox.Tag = macroElementID;
                        localRootSignaturesMacroNameShaderTextBox.TextChanged +=
                            new TextChangedEventHandler(AddLocalRootSignatureMacroNameShaderByID);
                    }
                    if (shouldAssociateLocalRootSignatureExportsTextBoxToJSON)
                    {
                        localRootSignaturesExportsTextBox.Tag = macroElementID;
                        localRootSignaturesExportsTextBox.TextChanged +=
                            new TextChangedEventHandler(AddLocalRootSignatureExportByID);
                    }
                    if (shouldAssociateLocalRootSignatureTypeCobmoBoxToJSON)
                    {
                        localRootSignaturesTypeComboBox.Tag = macroElementID;
                        localRootSignaturesTypeComboBox.SelectionChanged +=
                            new SelectionChangedEventHandler(AddLocalRootSignatureTypeByID);
                        Mode previousApplicationMode = CurrentApplicationMode;
                        SetApplicationMode(Mode.LocalRootSignature);
                        localRootSignaturesTypeComboBox.SelectedValue = localRootSignaturesTypeComboBox.Items[0];
                        SetApplicationMode(previousApplicationMode);
                    }
                }
                else
                {
                    shouldConnectButtonEvent = false;
                    postErrorMessage("Unable to create " + nameof(localRootSignaturesModeRootElement) + nameModifier);
                    success = false;
                }
                MacroElement macroElement;
                if (success)
                {
                    ModeCount.LocalRootSignatureCount++;
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.LocalRootSignature,
                        rootElement = localRootSignaturesModeRootElement,
                        targetElement = FindName("contentGrid") as Panel,
                        regiteredChildrenNames = nameList,
                        hasItemSeperator = true
                    };
                }
                else
                {
                    postErrorMessage("Unable to create element for mode: " + nameof(Mode.LocalRootSignature));
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.ApplicationError,
                        rootElement = null,
                        targetElement = null,
                        regiteredChildrenNames = null
                    };
                }
                return macroElement;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create the local root signature configuration pane: " +
                    ex.ToString());
                return new MacroElement();
            }
        }
        public MacroElement CreateGlobalRootSignatureConfig()
        {
            return CreateGlobalRootSignatureConfig(0);
        }
        public MacroElement CreateGlobalRootSignatureConfig(UInt32 nameModifier)
        {
            try
            {
                List<String> nameList = new List<string>();
                bool success = true;
                //
                // Type elements.
                //
                bool shouldAssociateGlobalRootSignaturesTypeComboBoxToJSON = true;
                ComboBox globalRootSignaturesTypeComboBox = new ComboBox()
                {
                    Margin = new Thickness(ContentViewSettings.ContentComboBoxMargin_left,
                        ContentViewSettings.ContentComboBoxMargin_top,
                        ContentViewSettings.ContentComboBoxMargin_right,
                        ContentViewSettings.ContentComboBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MinWidth = ContentViewSettings.ContentComboBoxMinWidth
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesTypeComboBox, nameof(globalRootSignaturesTypeComboBox) + nameModifier, nameList))
                {
                    globalRootSignaturesTypeComboBox.SetValue(Grid.RowProperty, 0);
                    globalRootSignaturesTypeComboBox.SetValue(Grid.ColumnProperty, 1);
                    globalRootSignaturesTypeComboBox.Style = FindResource("baseComboBoxStyle") as Style;
                    List<String> globalRootSignatureTypeComboBoxItems =
                        new List<String>(new String[]
                        {
                            ConstStringBinaryDXILType
                        });

                    SetComboBoxItemContent(globalRootSignaturesTypeComboBox, globalRootSignatureTypeComboBoxItems);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesTypeComboBox) + nameModifier);
                    shouldAssociateGlobalRootSignaturesTypeComboBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label globalRootSignaturesTypeLabel = new Label()
                {
                    Content = "Type",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(globalRootSignaturesTypeComboBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesTypeLabel, nameof(globalRootSignaturesTypeLabel) + nameModifier, nameList))
                {
                    globalRootSignaturesTypeLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesTypeLabel) + nameModifier);
                    success = false;
                }
                StackPanel globalRootSignaturesTypeStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesTypeStackPanel, nameof(globalRootSignaturesTypeStackPanel) + nameModifier, nameList))
                {
                    globalRootSignaturesTypeStackPanel.Children.Add(globalRootSignaturesTypeLabel);
                    globalRootSignaturesTypeStackPanel.Children.Add(globalRootSignaturesTypeComboBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesTypeStackPanel) + nameModifier);
                    success = false;
                }
                //
                // File path elements.
                //
                bool shouldConnectglobalRootSignaturesFilePathBrowseButton = true;
                Button globalRootSignaturesFilePathBrowseButton = new Button()
                {
                    Style = FindResource("browseButtonStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentBrowseButtonMargin_left,
                        ContentViewSettings.ContentBrowseButtonMargin_top,
                        ContentViewSettings.ContentBrowseButtonMargin_right,
                        ContentViewSettings.ContentBrowseButtonMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    Width = ContentViewSettings.ContentBrowseButtonWidth
                };
                if (!WPFAssignAndRegisterName(globalRootSignaturesFilePathBrowseButton, nameof(globalRootSignaturesFilePathBrowseButton) + nameModifier, nameList))
                {
                    shouldConnectglobalRootSignaturesFilePathBrowseButton = false;
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesFilePathBrowseButton) + nameModifier);
                    success = false;
                }
                bool shouldAssociateGlobalRootSignatureFilepathTextBoxToJSON = true;
                TextBox globalRootSignaturesFilePathTextBox = new TextBox()
                {
                    Style = FindResource("defaultYetInvalidTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesFilePathTextBox, nameof(globalRootSignaturesFilePathTextBox) + nameModifier, nameList))
                {
                    if (shouldConnectglobalRootSignaturesFilePathBrowseButton)
                    {
                        globalRootSignaturesFilePathBrowseButton.Click += new RoutedEventHandler(BrowseForFilePath);
                        globalRootSignaturesFilePathBrowseButton.Tag = globalRootSignaturesFilePathTextBox.Name;
                    }
                    globalRootSignaturesFilePathTextBox.SetValue(Grid.ColumnProperty, 1);
                    globalRootSignaturesFilePathTextBox.ToolTip = globalRootSignaturesFilePathToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesFilePathTextBox) + nameModifier);
                    shouldAssociateGlobalRootSignatureFilepathTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label globalRootSignaturesFilePathLabel = new Label()
                {
                    Content = "File path",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(globalRootSignaturesFilePathTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesFilePathLabel, nameof(globalRootSignaturesFilePathLabel) + nameModifier, nameList))
                {
                    globalRootSignaturesFilePathLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesFilePathLabel) + nameModifier);
                    success = false;
                }
                StackPanel globalRootSignaturesFilePathStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesFilePathStackPanel, nameof(globalRootSignaturesFilePathStackPanel) + nameModifier, nameList))
                {
                    globalRootSignaturesFilePathStackPanel.Children.Add(globalRootSignaturesFilePathLabel);
                    globalRootSignaturesFilePathStackPanel.Children.Add(globalRootSignaturesFilePathTextBox);
                    globalRootSignaturesFilePathStackPanel.Children.Add(globalRootSignaturesFilePathBrowseButton);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesFilePathStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Macro Name shader elements.
                //
                bool shouldAssociateGlobalRootSignatureMacroNameShaderTextBoxToJSON = true;
                TextBox globalRootSignaturesMacroNameShaderTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesMacroNameShaderTextBox, nameof(globalRootSignaturesMacroNameShaderTextBox) + nameModifier, nameList))
                {
                    globalRootSignaturesMacroNameShaderTextBox.SetValue(Grid.RowProperty, 0);
                    globalRootSignaturesMacroNameShaderTextBox.SetValue(Grid.ColumnProperty, 1);
                    globalRootSignaturesMacroNameShaderTextBox.ToolTip = globalRootSignaturesMacroNameShaderToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesMacroNameShaderTextBox) + nameModifier);
                    shouldAssociateGlobalRootSignatureMacroNameShaderTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label globalRootSignaturesMacroNameShaderLabel = new Label()
                {
                    Content = "Macro name (HLSL)",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(globalRootSignaturesMacroNameShaderTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesMacroNameShaderLabel, nameof(globalRootSignaturesMacroNameShaderLabel) + nameModifier, nameList))
                {
                    globalRootSignaturesMacroNameShaderLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesMacroNameShaderLabel) + nameModifier);
                    success = false;
                }
                StackPanel globalRootSignaturesMacroNameShaderStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top,
                    // Note: Needs to be visible/settable in the JSON representation, but not
                    //       editable by the user for the time being.
                    Visibility = Visibility.Collapsed
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesMacroNameShaderStackPanel, nameof(globalRootSignaturesMacroNameShaderStackPanel) + nameModifier, nameList))
                {
                    globalRootSignaturesMacroNameShaderStackPanel.Children.Add(globalRootSignaturesMacroNameShaderLabel);
                    globalRootSignaturesMacroNameShaderStackPanel.Children.Add(globalRootSignaturesMacroNameShaderTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesMacroNameShaderStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Exports elements.
                //
                bool shouldAssociateGlobalRootSignatureExportsTextBoxToJSON = true;
                TextBox globalRootSignaturesExportsTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesExportsTextBox, nameof(globalRootSignaturesExportsTextBox) + nameModifier, nameList))
                {
                    globalRootSignaturesExportsTextBox.SetValue(Grid.RowProperty, 0);
                    globalRootSignaturesExportsTextBox.SetValue(Grid.ColumnProperty, 1);
                    globalRootSignaturesExportsTextBox.ToolTip = globalRootSignaturesExportsToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesExportsTextBox) + nameModifier);
                    shouldAssociateGlobalRootSignatureExportsTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label globalRootSignaturesExportsLabel = new Label()
                {
                    Content = "Exports",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(globalRootSignaturesExportsTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesExportsLabel, nameof(globalRootSignaturesExportsLabel) + nameModifier, nameList))
                {
                    globalRootSignaturesExportsLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesExportsLabel) + nameModifier);
                    success = false;
                }
                StackPanel globalRootSignaturesExportsStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesExportsStackPanel, nameof(globalRootSignaturesExportsStackPanel) + nameModifier, nameList))
                {
                    globalRootSignaturesExportsStackPanel.Children.Add(globalRootSignaturesExportsLabel);
                    globalRootSignaturesExportsStackPanel.Children.Add(globalRootSignaturesExportsTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesExportsStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Vertical Stack Panel
                //
                StackPanel globalRootSignaturesVerticalStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Vertical,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top,
                    MinWidth = ContentViewSettings.ContentMinWidth
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesVerticalStackPanel, nameof(globalRootSignaturesVerticalStackPanel) + nameModifier, nameList))
                {
                    // Add separator.
                    CreateMacroElementSeparator(globalRootSignaturesVerticalStackPanel);
                    globalRootSignaturesVerticalStackPanel.Children.Add(globalRootSignaturesTypeStackPanel);
                    globalRootSignaturesVerticalStackPanel.Children.Add(globalRootSignaturesFilePathStackPanel);
                    globalRootSignaturesVerticalStackPanel.Children.Add(globalRootSignaturesMacroNameShaderStackPanel);
                    globalRootSignaturesVerticalStackPanel.Children.Add(globalRootSignaturesExportsStackPanel);
                    globalRootSignaturesVerticalStackPanel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesVerticalStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Top level elements.
                //
                bool shouldConnectButtonEvent = true;
                Button globalRootSignaturesRemoveItemButton = new Button()
                {
                    Style = FindResource("removeItemButtonStyle") as Style,
                    Content = GetRemoveButtonTrashCanIcon(),
                    Margin = new Thickness(ContentViewSettings.ContentRemoveButton_withoutBrowse_withTextBox_MarginLeft, 0, 0,
                    ContentViewSettings.ContentRemoveButton_withoutBrowse_withTextBox_MarginBottom)
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesRemoveItemButton, nameof(globalRootSignaturesRemoveItemButton) + nameModifier, nameList))
                {
                    globalRootSignaturesRemoveItemButton.SetValue(Grid.ColumnProperty, 1);
                    globalRootSignaturesRemoveItemButton.ToolTip = removeButtonToolTipPartialString + GetModeAsText(Mode.GlobalRootSignature);
                }
                else
                {
                    shouldConnectButtonEvent = false;
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesRemoveItemButton) + nameModifier);
                    success = false;
                }
                ColumnDefinition globalRootSignaturesLabelAndTextboxCol = new ColumnDefinition()
                {
                    Width = new GridLength(1.0, GridUnitType.Auto),
                    MinWidth = 100
                };
                if (!WPFAssignAndRegisterName(globalRootSignaturesLabelAndTextboxCol, nameof(globalRootSignaturesLabelAndTextboxCol) + nameModifier, nameList))
                {
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesLabelAndTextboxCol) + nameModifier);
                    success = false;
                }
                Grid globalRootSignaturesModeRootElement = new Grid()
                {
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(globalRootSignaturesModeRootElement, nameof(globalRootSignaturesModeRootElement) + nameModifier, nameList))
                {
                    String macroElementID = nameof(globalRootSignaturesModeRootElement) + nameModifier;
                    globalRootSignaturesModeRootElement.ColumnDefinitions.Add(globalRootSignaturesLabelAndTextboxCol);
                    globalRootSignaturesModeRootElement.Children.Add(globalRootSignaturesVerticalStackPanel);
                    globalRootSignaturesModeRootElement.Tag = macroElementID;
                    // Associate button mechanics.
                    if (shouldConnectButtonEvent)
                    {
                        // Tuple layout: <parentName, grandParentName>
                        globalRootSignaturesRemoveItemButton.Tag = macroElementID;
                        globalRootSignaturesRemoveItemButton.Click += new RoutedEventHandler(HandleTrashcanIconClick);
                        globalRootSignaturesTypeStackPanel.Children.Add(globalRootSignaturesRemoveItemButton);
                    }
                    // Associate JSON mechanics.
                    if (shouldAssociateGlobalRootSignatureFilepathTextBoxToJSON)
                    {
                        globalRootSignaturesFilePathTextBox.Tag = macroElementID;
                        globalRootSignaturesFilePathTextBox.Text = defaultYetInvalidFilePathString;
                        globalRootSignaturesFilePathTextBox.TextChanged +=
                            new TextChangedEventHandler(AddGlobalRootSignatureFilePathByID);
                        globalRootSignaturesFilePathTextBox.GotFocus += ClearDefaultFilePathText;
                    }
                    if (shouldAssociateGlobalRootSignatureMacroNameShaderTextBoxToJSON)
                    {
                        globalRootSignaturesMacroNameShaderTextBox.Tag = macroElementID;
                        globalRootSignaturesMacroNameShaderTextBox.TextChanged +=
                            new TextChangedEventHandler(AddGlobalRootSignatureMacroNameShaderByID);
                    }
                    if (shouldAssociateGlobalRootSignatureExportsTextBoxToJSON)
                    {
                        globalRootSignaturesExportsTextBox.Tag = macroElementID;
                        globalRootSignaturesExportsTextBox.TextChanged +=
                            new TextChangedEventHandler(AddGlobalRootSignatureExportByID);
                    }
                    if (shouldAssociateGlobalRootSignaturesTypeComboBoxToJSON)
                    {
                        globalRootSignaturesTypeComboBox.Tag = macroElementID;
                        globalRootSignaturesTypeComboBox.SelectionChanged +=
                            new SelectionChangedEventHandler(AddGlobalRootSignatureTypeByID);
                        Mode previousApplicationMode = CurrentApplicationMode;
                        SetApplicationMode(Mode.GlobalRootSignature);
                        globalRootSignaturesTypeComboBox.SelectedValue = globalRootSignaturesTypeComboBox.Items[0];
                        SetApplicationMode(previousApplicationMode);
                    }
                }
                else
                {
                    shouldConnectButtonEvent = false;
                    postErrorMessage("Unable to create " + nameof(globalRootSignaturesModeRootElement) + nameModifier);
                    success = false;
                }
                MacroElement macroElement;
                if (success)
                {
                    ModeCount.GlobalRootSignatureCount++;
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.GlobalRootSignature,
                        rootElement = globalRootSignaturesModeRootElement,
                        targetElement = FindName("contentGrid") as Panel,
                        regiteredChildrenNames = nameList,
                        hasItemSeperator = true
                    };
                }
                else
                {
                    postErrorMessage("Unable to create element for mode: " + nameof(Mode.GlobalRootSignature));
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.ApplicationError,
                        rootElement = null,
                        targetElement = null,
                        regiteredChildrenNames = null
                    };
                }
                return macroElement;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create the global root signature configuration pane: " +
                    ex.ToString());
                return new MacroElement();
            }
        }
        public MacroElement CreateRaytracingPipelineConfig()
        {
            return CreateRaytracingPipelineConfig(0);
        }
        public MacroElement CreateRaytracingPipelineConfig(UInt32 nameModifier)
        {
            try
            {
                //SetApplicationMode(Mode.RaytracingPipeline);
                List<String> nameList = new List<string>();
                bool success = true;
                //
                // Max Trace Recursion Depth elements.
                //
                bool shouldAssociateRaytracingPipelineMaxTraceRecursionTextBoxToJSON = true;
                TextBox RaytracingPipelineMaxTraceRecursionDepthTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth
                };
                if (WPFAssignAndRegisterName(RaytracingPipelineMaxTraceRecursionDepthTextBox, nameof(RaytracingPipelineMaxTraceRecursionDepthTextBox) + nameModifier, nameList))
                {
                    RaytracingPipelineMaxTraceRecursionDepthTextBox.SetValue(Grid.RowProperty, 0);
                    RaytracingPipelineMaxTraceRecursionDepthTextBox.SetValue(Grid.ColumnProperty, 1);
                    RaytracingPipelineMaxTraceRecursionDepthTextBox.ToolTip = RaytracingPipelineMaxTraceRecursionDepthToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(RaytracingPipelineMaxTraceRecursionDepthTextBox) + nameModifier);
                    shouldAssociateRaytracingPipelineMaxTraceRecursionTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label RaytracingPipelineMaxTraceRecursionDepthLabel = new Label()
                {
                    Content = "Max trace recursion depth",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(RaytracingPipelineMaxTraceRecursionDepthTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(RaytracingPipelineMaxTraceRecursionDepthLabel, nameof(RaytracingPipelineMaxTraceRecursionDepthLabel) + nameModifier, nameList))
                {
                    RaytracingPipelineMaxTraceRecursionDepthLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(RaytracingPipelineMaxTraceRecursionDepthLabel) + nameModifier);
                    success = false;
                }
                StackPanel RaytracingPipelineMaxTraceRecursionDepthStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(RaytracingPipelineMaxTraceRecursionDepthStackPanel, nameof(RaytracingPipelineMaxTraceRecursionDepthStackPanel) + nameModifier, nameList))
                {
                    RaytracingPipelineMaxTraceRecursionDepthStackPanel.Children.Add(RaytracingPipelineMaxTraceRecursionDepthLabel);
                    RaytracingPipelineMaxTraceRecursionDepthStackPanel.Children.Add(RaytracingPipelineMaxTraceRecursionDepthTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(RaytracingPipelineMaxTraceRecursionDepthStackPanel) + nameModifier);
                    success = false;
                }

                //
                // Exports elements.
                //
                bool shouldAssociateRaytracingPipelineExportsTextBoxToJSON = true;
                TextBox RaytracingPipelineExportsTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth
                };
                if (WPFAssignAndRegisterName(RaytracingPipelineExportsTextBox, nameof(RaytracingPipelineExportsTextBox) + nameModifier, nameList))
                {
                    RaytracingPipelineExportsTextBox.SetValue(Grid.RowProperty, 0);
                    RaytracingPipelineExportsTextBox.SetValue(Grid.ColumnProperty, 1);
                    RaytracingPipelineExportsTextBox.ToolTip = RaytracingPipelineExportsToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(RaytracingPipelineExportsTextBox) + nameModifier);
                    shouldAssociateRaytracingPipelineExportsTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label RaytracingPipelineExportsLabel = new Label()
                {
                    Content = "Exports",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(RaytracingPipelineExportsTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(RaytracingPipelineExportsLabel, nameof(RaytracingPipelineExportsLabel) + nameModifier, nameList))
                {
                    RaytracingPipelineExportsLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(RaytracingPipelineExportsLabel) + nameModifier);
                    success = false;
                }
                StackPanel RaytracingPipelineExportsStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(RaytracingPipelineExportsStackPanel, nameof(RaytracingPipelineExportsStackPanel) + nameModifier, nameList))
                {
                    RaytracingPipelineExportsStackPanel.Children.Add(RaytracingPipelineExportsLabel);
                    RaytracingPipelineExportsStackPanel.Children.Add(RaytracingPipelineExportsTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(RaytracingPipelineExportsStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Vertical Stack Panel
                //
                StackPanel RaytracingPipelineVerticalStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Vertical,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top,
                    MinWidth = ContentViewSettings.ContentMinWidth
                };
                if (WPFAssignAndRegisterName(RaytracingPipelineVerticalStackPanel, nameof(RaytracingPipelineVerticalStackPanel) + nameModifier, nameList))
                {
                    RaytracingPipelineVerticalStackPanel.Children.Add(RaytracingPipelineMaxTraceRecursionDepthStackPanel);
                    // Note: Restore if RaytracingPipelineConfig->Flags element is reinstated.
                    //RaytracingPipelineVerticalStackPanel.Children.Add(RaytracingPipelineFlagsStackPanel);
                    RaytracingPipelineVerticalStackPanel.Children.Add(RaytracingPipelineExportsStackPanel);
                    RaytracingPipelineVerticalStackPanel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(RaytracingPipelineVerticalStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Top level elements.
                //
                ColumnDefinition RaytracingPipelineLabelAndTextboxCol = new ColumnDefinition()
                {
                    Width = new GridLength(1.0, GridUnitType.Auto),
                    MinWidth = 100
                };
                if (!WPFAssignAndRegisterName(RaytracingPipelineLabelAndTextboxCol, nameof(RaytracingPipelineLabelAndTextboxCol) + nameModifier, nameList))
                {
                    postErrorMessage("Unable to create " + nameof(RaytracingPipelineLabelAndTextboxCol) + nameModifier);
                    success = false;
                }
                Grid RaytracingPipelineModeRootElement = new Grid()
                {
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(RaytracingPipelineModeRootElement, nameof(RaytracingPipelineModeRootElement) + nameModifier, nameList))
                {
                    RaytracingPipelineModeRootElement.ColumnDefinitions.Add(RaytracingPipelineLabelAndTextboxCol);
                    RaytracingPipelineModeRootElement.Children.Add(RaytracingPipelineVerticalStackPanel);
                    // Associate JSON Mechanics
                    if (shouldAssociateRaytracingPipelineMaxTraceRecursionTextBoxToJSON)
                    {
                        RaytracingPipelineMaxTraceRecursionDepthTextBox.TextChanged +=
                            new TextChangedEventHandler(EditRaytracingPipelineMaxRecursionDepthByID);
                        Mode previousApplicationMode = CurrentApplicationMode;
                        SetApplicationMode(Mode.RaytracingPipeline);
                        RaytracingPipelineMaxTraceRecursionDepthTextBox.Text = "1";
                        SetApplicationMode(previousApplicationMode);
                    }
                    // Note: Restore if RaytracingPipelineConfig->Flags element is reinstated.
                    //if (shouldAssociateRaytracingPipelineFlagsTextBoxToJSON)
                    //{
                    //    RaytracingPipelineFlagsTextBox.TextChanged +=
                    //        new TextChangedEventHandler(EditRaytracingPipelineFlagsByID);
                    //}
                    if (shouldAssociateRaytracingPipelineExportsTextBoxToJSON)
                    {
                        RaytracingPipelineExportsTextBox.TextChanged +=
                            new TextChangedEventHandler(EditRaytracingPipelineExportsByID);
                    }
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(RaytracingPipelineModeRootElement) + nameModifier);
                    success = false;
                }
                MacroElement macroElement;
                if (success)
                {
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.RaytracingPipeline,
                        rootElement = RaytracingPipelineModeRootElement,
                        targetElement = FindName("contentGrid") as Panel,
                        regiteredChildrenNames = nameList
                    };
                }
                else
                {
                    postErrorMessage("Unable to create element for mode: " + nameof(Mode.RaytracingPipeline));
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.ApplicationError,
                        rootElement = null,
                        targetElement = null,
                        regiteredChildrenNames = null,
                        hasItemSeperator = false
                    };
                }
                return macroElement;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create the raytracing pipeline configuration pane: " +
                    ex.ToString());
                return new MacroElement();
            }
        }
        public MacroElement CreateShaderPipelineConfig()
        {
            return CreateShaderPipelineConfig(0);
        }
        public MacroElement CreateShaderPipelineConfig(UInt32 nameModifier)
        {
            try
            {
                List<String> nameList = new List<string>();
                bool success = true;
                //
                // Payload Size elements.
                //
                bool shouldAssociateShaderPipelineTextBoxToJSON = true;
                TextBox ShaderPipelinePayloadSizeTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth
                };
                if (WPFAssignAndRegisterName(ShaderPipelinePayloadSizeTextBox, nameof(ShaderPipelinePayloadSizeTextBox) + nameModifier, nameList))
                {
                    ShaderPipelinePayloadSizeTextBox.SetValue(Grid.ColumnProperty, 1);
                    ShaderPipelinePayloadSizeTextBox.ToolTip = ShaderPipelinePayloadSizeToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinePayloadSizeTextBox) + nameModifier);
                    shouldAssociateShaderPipelineTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label ShaderPipelinePayloadSizeLabel = new Label()
                {
                    Content = "Payload size (bytes)",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(ShaderPipelinePayloadSizeTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(ShaderPipelinePayloadSizeLabel, nameof(ShaderPipelinePayloadSizeLabel) + nameModifier, nameList))
                {
                    ShaderPipelinePayloadSizeLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinePayloadSizeLabel) + nameModifier);
                    success = false;
                }
                StackPanel ShaderPipelinePayloadSizeStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(ShaderPipelinePayloadSizeStackPanel, nameof(ShaderPipelinePayloadSizeStackPanel) + nameModifier, nameList))
                {
                    ShaderPipelinePayloadSizeStackPanel.Children.Add(ShaderPipelinePayloadSizeLabel);
                    ShaderPipelinePayloadSizeStackPanel.Children.Add(ShaderPipelinePayloadSizeTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinePayloadSizeStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Max AttributeSize elements.
                //
                bool shouldAssociateShaderPipelineMaxAttributeSizeTextBoxToJSON = true;
                TextBox ShaderPipelinesMaxAttributeSizeTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth
                };
                if (WPFAssignAndRegisterName(ShaderPipelinesMaxAttributeSizeTextBox, nameof(ShaderPipelinesMaxAttributeSizeTextBox) + nameModifier, nameList))
                {
                    ShaderPipelinesMaxAttributeSizeTextBox.SetValue(Grid.RowProperty, 0);
                    ShaderPipelinesMaxAttributeSizeTextBox.SetValue(Grid.ColumnProperty, 1);
                    ShaderPipelinesMaxAttributeSizeTextBox.ToolTip = ShaderPipelinesMaxAttributeSizeToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinesMaxAttributeSizeTextBox) + nameModifier);
                    shouldAssociateShaderPipelineMaxAttributeSizeTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label ShaderPipelinesMaxAttributeSizeLabel = new Label()
                {
                    Content = "Max attribute size (bytes)",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(ShaderPipelinesMaxAttributeSizeTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(ShaderPipelinesMaxAttributeSizeLabel, nameof(ShaderPipelinesMaxAttributeSizeLabel) + nameModifier, nameList))
                {
                    ShaderPipelinesMaxAttributeSizeLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinesMaxAttributeSizeLabel) + nameModifier);
                    success = false;
                }
                StackPanel ShaderPipelinesMaxAttributeSizeStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(ShaderPipelinesMaxAttributeSizeStackPanel, nameof(ShaderPipelinesMaxAttributeSizeStackPanel) + nameModifier, nameList))
                {
                    ShaderPipelinesMaxAttributeSizeStackPanel.Children.Add(ShaderPipelinesMaxAttributeSizeLabel);
                    ShaderPipelinesMaxAttributeSizeStackPanel.Children.Add(ShaderPipelinesMaxAttributeSizeTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinesMaxAttributeSizeStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Exports elements.
                //
                bool shouldAssociateRaytracingPipelineExportsTextBoxToJSON = true;
                TextBox ShaderPipelinesExportsTextBox = new TextBox()
                {
                    Style = FindResource("baseTextBoxStyle") as Style,
                    Margin = new Thickness(ContentViewSettings.ContentTextBoxMargin_left,
                        ContentViewSettings.ContentTextBoxMargin_top,
                        ContentViewSettings.ContentTextBoxMargin_right,
                        ContentViewSettings.ContentTextBoxMargin_bottom),
                    Height = ContentViewSettings.ContentItemHeight,
                    MaxWidth = ContentViewSettings.ContentTextBoxMaxWidth,
                    MinWidth = ContentViewSettings.ContentTextBoxMinWidth
                };
                if (WPFAssignAndRegisterName(ShaderPipelinesExportsTextBox, nameof(ShaderPipelinesExportsTextBox) + nameModifier, nameList))
                {
                    ShaderPipelinesExportsTextBox.SetValue(Grid.RowProperty, 0);
                    ShaderPipelinesExportsTextBox.SetValue(Grid.ColumnProperty, 1);
                    ShaderPipelinesExportsTextBox.ToolTip = ShaderPipelinesExportsToolTipString;
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinesExportsTextBox) + nameModifier);
                    shouldAssociateRaytracingPipelineExportsTextBoxToJSON = false;
                    success = false;
                }
                // Note: Must build and register corresponding text box first,
                //       in order to set the target of this label.
                Label ShaderPipelinesExportsLabel = new Label()
                {
                    Content = "Exports",
                    Style = FindResource("baseLabelStyle") as Style,
                    Target = FindName(nameof(ShaderPipelinesExportsTextBox)) as UIElement
                };
                if (WPFAssignAndRegisterName(ShaderPipelinesExportsLabel, nameof(ShaderPipelinesExportsLabel) + nameModifier, nameList))
                {
                    ShaderPipelinesExportsLabel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinesExportsLabel) + nameModifier);
                    success = false;
                }
                StackPanel ShaderPipelinesExportsStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(ShaderPipelinesExportsStackPanel, nameof(ShaderPipelinesExportsStackPanel) + nameModifier, nameList))
                {
                    ShaderPipelinesExportsStackPanel.Children.Add(ShaderPipelinesExportsLabel);
                    ShaderPipelinesExportsStackPanel.Children.Add(ShaderPipelinesExportsTextBox);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinesExportsStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Vertical Stack Panel
                //
                StackPanel ShaderPipelinesVerticalStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Vertical,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top,
                    MinWidth = ContentViewSettings.ContentMinWidth
                };
                if (WPFAssignAndRegisterName(ShaderPipelinesVerticalStackPanel, nameof(ShaderPipelinesVerticalStackPanel) + nameModifier, nameList))
                {
                    // Add separator.
                    CreateMacroElementSeparator(ShaderPipelinesVerticalStackPanel);
                    ShaderPipelinesVerticalStackPanel.Children.Add(ShaderPipelinePayloadSizeStackPanel);
                    ShaderPipelinesVerticalStackPanel.Children.Add(ShaderPipelinesMaxAttributeSizeStackPanel);
                    ShaderPipelinesVerticalStackPanel.Children.Add(ShaderPipelinesExportsStackPanel);
                    ShaderPipelinesVerticalStackPanel.SetValue(Grid.ColumnProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinesVerticalStackPanel) + nameModifier);
                    success = false;
                }
                //
                // Top level elements.
                //
                bool shouldConnectButtonEvent = true;
                Button ShaderPipelinesRemoveItemButton = new Button()
                {
                    Style = FindResource("removeItemButtonStyle") as Style,
                    Content = GetRemoveButtonTrashCanIcon(),
                    Margin = new Thickness(ContentViewSettings.ContentRemoveButton_withoutBrowse_withTextBox_MarginLeft, 0, 0,
                    ContentViewSettings.ContentRemoveButton_withoutBrowse_withTextBox_MarginBottom)
                };
                if (WPFAssignAndRegisterName(ShaderPipelinesRemoveItemButton, nameof(ShaderPipelinesRemoveItemButton) + nameModifier, nameList))
                {
                    ShaderPipelinesRemoveItemButton.SetValue(Grid.ColumnProperty, 1);
                    ShaderPipelinesRemoveItemButton.ToolTip = removeButtonToolTipPartialString + GetModeAsText(Mode.ShaderPipeline) + " config";
                }
                else
                {
                    shouldConnectButtonEvent = false;
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinesRemoveItemButton) + nameModifier);
                    success = false;
                }
                ColumnDefinition ShaderPipelinesLabelAndTextboxCol = new ColumnDefinition()
                {
                    Width = new GridLength(1.0, GridUnitType.Auto),
                    MinWidth = 100
                };
                if (!WPFAssignAndRegisterName(ShaderPipelinesLabelAndTextboxCol, nameof(ShaderPipelinesLabelAndTextboxCol) + nameModifier, nameList))
                {
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinesLabelAndTextboxCol) + nameModifier);
                    success = false;
                }
                Grid ShaderPipelinesModeRootElement = new Grid()
                {
                    HorizontalAlignment = HorizontalAlignment.Left,
                    VerticalAlignment = VerticalAlignment.Top
                };
                if (WPFAssignAndRegisterName(ShaderPipelinesModeRootElement, nameof(ShaderPipelinesModeRootElement) + nameModifier, nameList))
                {
                    String macroElementID = nameof(ShaderPipelinesModeRootElement) + nameModifier;
                    ShaderPipelinesModeRootElement.ColumnDefinitions.Add(ShaderPipelinesLabelAndTextboxCol);
                    ShaderPipelinesModeRootElement.Children.Add(ShaderPipelinesVerticalStackPanel);
                    ShaderPipelinesModeRootElement.Tag = macroElementID;
                    // Associate button mechanics.
                    if (shouldConnectButtonEvent)
                    {
                        // Tuple layout: <parentName, grandParentName>
                        ShaderPipelinesRemoveItemButton.Tag = macroElementID;
                        ShaderPipelinesRemoveItemButton.Click += new RoutedEventHandler(HandleTrashcanIconClick);
                        ShaderPipelinePayloadSizeStackPanel.Children.Add(ShaderPipelinesRemoveItemButton);
                    }
                    // Associate JSON Mechanics
                    if (shouldAssociateShaderPipelineTextBoxToJSON)
                    {
                        ShaderPipelinePayloadSizeTextBox.Tag = macroElementID;
                        ShaderPipelinePayloadSizeTextBox.TextChanged +=
                            new TextChangedEventHandler(AddRaytracingShaderConfigMaxPayloadSizeByID);
                        Mode previousApplicationMode = CurrentApplicationMode;
                        SetApplicationMode(Mode.ShaderPipeline);
                        ShaderPipelinePayloadSizeTextBox.Text = "16";
                        SetApplicationMode(previousApplicationMode);
                    }
                    if (shouldAssociateShaderPipelineMaxAttributeSizeTextBoxToJSON)
                    {
                        ShaderPipelinesMaxAttributeSizeTextBox.Tag = macroElementID;
                        ShaderPipelinesMaxAttributeSizeTextBox.TextChanged +=
                            new TextChangedEventHandler(AddRaytracingShaderConfigMaxAttributeSizeByID);
                        Mode previousApplicationMode = CurrentApplicationMode;
                        SetApplicationMode(Mode.ShaderPipeline);
                        ShaderPipelinesMaxAttributeSizeTextBox.Text = "8";
                        SetApplicationMode(previousApplicationMode);
                    }
                    if (shouldAssociateRaytracingPipelineExportsTextBoxToJSON)
                    {
                        ShaderPipelinesExportsTextBox.Tag = macroElementID;
                        ShaderPipelinesExportsTextBox.TextChanged +=
                            new TextChangedEventHandler(AddRaytracingShaderConfigExportByID);
                    }
                }
                else
                {
                    shouldConnectButtonEvent = false;
                    postErrorMessage("Unable to create " + nameof(ShaderPipelinesModeRootElement) + nameModifier);
                    success = false;
                }
                MacroElement macroElement;
                if (success)
                {
                    ModeCount.ShaderPipelineCount++;
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.ShaderPipeline,
                        rootElement = ShaderPipelinesModeRootElement,
                        targetElement = FindName("contentGrid") as Panel,
                        regiteredChildrenNames = nameList,
                        hasItemSeperator = true
                    };
                }
                else
                {
                    postErrorMessage("Unable to create element for mode: " + nameof(Mode.ShaderPipeline));
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.ApplicationError,
                        rootElement = null,
                        targetElement = null,
                        regiteredChildrenNames = null
                    };
                }
                return macroElement;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create the shader pipeline configuration pane: " +
                    ex.ToString());
                return new MacroElement();
            }
        }
        public MacroElement CreateOutputConfig()
        {
            try
            {
                //SetApplicationMode(Mode.Output);
                List<String> nameList = new List<String>();
                bool success = true;
                RowDefinition jsonRow = new RowDefinition();
                if (!WPFAssignAndRegisterName(jsonRow, nameof(jsonRow), nameList))
                {
                    postErrorMessage("Unable to create " + nameof(jsonRow));
                    success = false;
                }
                // JSON Save Button
                Button saveButton = new Button()
                {
                    Content = "Save as",
                    Style = FindResource("primaryButtonStyle") as Style,
                    MinWidth = 135,
                    MinHeight = 35,
                    Margin = new Thickness(5, 5, 5, 5),
                    HorizontalAlignment = HorizontalAlignment.Right
                };
                if (WPFAssignAndRegisterName(saveButton, nameof(saveButton), nameList))
                {
                    saveButton.Click += new RoutedEventHandler(SaveButton_Click);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(saveButton));
                    success = false;
                }
                // JSON Load File Button
                Button loadJsonFileButton = new Button()
                {
                    Content = "Load",
                    Style = FindResource("primaryButtonStyle") as Style,
                    MinWidth = 135,
                    MinHeight = 35,
                    Margin = new Thickness(5, 5, 5, 5),
                    HorizontalAlignment = HorizontalAlignment.Right
                };
                if (WPFAssignAndRegisterName(loadJsonFileButton, nameof(loadJsonFileButton), nameList))
                {
                    loadJsonFileButton.Tag = "Load JSON From File";
                    loadJsonFileButton.Click += new RoutedEventHandler(HandleLoadJSONButton);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(loadJsonFileButton));
                    success = false;
                }
                StackPanel buttonPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Stretch
                };
                if (WPFAssignAndRegisterName(buttonPanel, nameof(buttonPanel), nameList))
                {
                    buttonPanel.SetValue(Grid.RowProperty, 1);
                    //buttonPanel.Children.Add(resetJsonButton);
                    buttonPanel.Children.Add(saveButton);
                    buttonPanel.Children.Add(loadJsonFileButton);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(loadJsonFileButton));
                    success = false;
                }
                TextBox jsonViewText = new TextBox()
                {
                    Style = FindResource("jsonTextStyle") as Style,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Stretch,
                    HorizontalContentAlignment = HorizontalAlignment.Left,
                    VerticalContentAlignment = VerticalAlignment.Top,
                    TextWrapping = TextWrapping.Wrap,
                    IsReadOnly = true,
                };
                if (WPFAssignAndRegisterName(jsonViewText, nameof(jsonViewText), nameList))
                {
                    jsonViewText.SetValue(Grid.RowProperty, 0);
                    //jsonViewText.Drop += new DragEventHandler(HandleJSONDragAndDrop);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(jsonViewText));
                    success = false;
                }
                bool contentGridSuccess = true;
                Grid jsonContentGrid = new Grid()
                {
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Top,
                };
                if (WPFAssignAndRegisterName(jsonContentGrid, nameof(jsonContentGrid), nameList))
                {
                    jsonContentGrid.SetValue(Grid.RowProperty, 0);
                    jsonContentGrid.Children.Add(jsonViewText);
                }
                else
                {
                    contentGridSuccess = false;
                    postErrorMessage("Unable to create " + nameof(jsonContentGrid));
                    success = false;
                }
                DXRScrollViewer jsonConfigScrollViewer = new DXRScrollViewer()
                {
                    HorizontalScrollBarVisibility = ScrollBarVisibility.Auto,
                    VerticalScrollBarVisibility = ScrollBarVisibility.Auto,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Stretch,
                    VerticalContentAlignment = VerticalAlignment.Top,
                    Margin = new Thickness(0, 0, 0, 0)
                };
                if (WPFAssignAndRegisterName(jsonConfigScrollViewer, nameof(jsonConfigScrollViewer), nameList))
                {
                    jsonConfigScrollViewer.SetValue(Grid.ColumnProperty, 2);
                    jsonConfigScrollViewer.SetValue(Grid.RowProperty, 0);
                    if (contentGridSuccess)
                    {
                        jsonConfigScrollViewer.Content = jsonContentGrid;
                        jsonConfigScrollViewer.ScrollChanged +=
                            new ScrollChangedEventHandler(HandleDXRScrollViewerScrollChanged);
                    }
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(jsonConfigScrollViewer));
                    success = false;
                }
                Grid outputGrid = new Grid()
                {
                    Background = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#FF333333"))
                };
                if (WPFAssignAndRegisterName(outputGrid, nameof(outputGrid), nameList))
                {
                    outputGrid.SetValue(Grid.ColumnProperty, 2);
                    outputGrid.RowDefinitions.Add(jsonRow);
                    outputGrid.Children.Add(jsonConfigScrollViewer);
                    // Add default button row.
                    CreateMacroElementButtonRow(outputGrid, buttonPanel);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(outputGrid));
                    success = false;
                }
                MacroElement macroElement;
                if (success)
                {
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.Output,
                        rootElement = outputGrid,
                        targetElement = FindName("jsonViewGrid") as Panel,
                        regiteredChildrenNames = nameList
                    };
                }
                else
                {
                    postErrorMessage("Unable to create element for mode: " + nameof(Mode.Output));
                    macroElement = new MacroElement
                    {
                        applicationMode = Mode.ApplicationError,
                        rootElement = null,
                        targetElement = null,
                        regiteredChildrenNames = null
                    };
                }
                return macroElement;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to create the output pane: " +
                    ex.ToString());
                return new MacroElement();
            }
        }
    }
}