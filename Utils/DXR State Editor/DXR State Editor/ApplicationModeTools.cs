using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;

namespace DXR_State_Editor
{
    public enum Mode
    {
        Welcome,
        GeneralStateConfig,
        Shader,
        HitGroup,
        LocalRootSignature,
        GlobalRootSignature,
        RaytracingPipeline,
        ShaderPipeline,
        Output,
        ApplicationError
    }

    public partial class MainWindow : Window
    {
        public Mode CurrentApplicationMode = Mode.Welcome;

        private Button GetApplicationModeButtonByApplicationMode(Mode mode)
        {
            if (mode == Mode.Welcome ||
                mode == Mode.Output ||
                mode == Mode.ApplicationError)
            {
                return null;
            }

            Button appModeButton = null;
            // Note: Strings in FindName() function are static. Do not change without
            //       also changing x:Name attributes in UniformGrid "buttonGrid" within MainWindow.xaml
            switch (mode)
            {
                case Mode.Shader:
                    appModeButton = FindName("shaderButton") as Button;
                    break;
                case Mode.HitGroup:
                    appModeButton = FindName("hitGroupsButton") as Button;
                    break;
                case Mode.LocalRootSignature:
                    appModeButton = FindName("localRootSignaturesButton") as Button;
                    break;
                case Mode.GlobalRootSignature:
                    appModeButton = FindName("globalRootSignaturesButton") as Button;
                    break;
                case Mode.RaytracingPipeline:
                    appModeButton = FindName("RaytracingPipelineConfigButton") as Button;
                    break;
                case Mode.ShaderPipeline:
                    appModeButton = FindName("shaderPipelineConfigButton") as Button;
                    break;
            }

            return appModeButton;
        }

        private String GetModeDescriptorText(Mode mode)
        {
            String retString = "";
            if (mode == Mode.Output ||
                mode == Mode.Output ||
                mode == Mode.Welcome ||
                mode == Mode.RaytracingPipeline ||
                mode == Mode.ApplicationError)
            {
                return retString;
            }

            String description_part_one = "Click the Add new ";
            String description_part_two = " button or use Ctrl+N to create a new ";
            String modeString = GetModeAsText(mode);

            retString = description_part_one + modeString +
                description_part_two + modeString + ".";
            return retString;
        }

        public void SetApplicationMode(Mode mode)
        {
            try
            {
                // Set global mode.
                CurrentApplicationMode = mode;

                // Reset Button Styles (sending button will be Styled in click handler).
                UniformGrid buttonGrid = FindName("buttonGrid") as UniformGrid;
                if (buttonGrid != null)
                {
                    int buttonCount = VisualTreeHelper.GetChildrenCount(buttonGrid);
                    for (int index = 0; index < buttonCount; index++)
                    {
                        Button modeButton = VisualTreeHelper.GetChild(buttonGrid, index) as Button;
                        if (modeButton != null)
                        {

                            modeButton.Style = FindResource("AppModeButtonStyle") as Style;
                        }
                    }
                }
                else
                {
                    postWarningMessage("Could not reset button styles. Unable to obtain reference to button grid.");
                }

                // Decide if the general state configuration panel should be shown.
                bool enableStateConfig = true;
                bool enableStateConfigAddButton = true;
                if (mode == Mode.Welcome)
                {
                    enableStateConfig = false;
                }
                else if (mode == Mode.RaytracingPipeline)
                {
                    enableStateConfigAddButton = false;
                }

                Grid cGrid = FindName("contentGrid") as Grid;
                if (cGrid == null)
                {
                    postErrorMessage("A call to " + nameof(SetApplicationMode) +
                        " was unable to acquire the general content grid.");
                    return;
                }

                Button addButton = FindName("addButton") as Button;
                if (addButton == null)
                {
                    postErrorMessage("A call to " + nameof(SetApplicationMode) +
                        " was unable to acquire the 'addButton' element from the General State Config.");
                    return;
                }

                String buttonContentString = "Add new " + GetModeAsText(mode);
                if (mode == Mode.ShaderPipeline)
                {
                    buttonContentString += " config";
                }

                addButton.Content = buttonContentString;
                addButton.Width = GetAddButtonWidthByMode(mode);

                TextBlock quickTitle = FindName("quickTitle") as TextBlock;
                if (quickTitle == null)
                {
                    postErrorMessage("Unalbe to set the quick title. The element cannot be found.");
                    return;
                }

                // Hide all FrameworkElements not belonging to the current application mode.
                foreach (var element in Master_MacroElementList)
                {
                    Mode elementApplicationMode = element.applicationMode;
                    if (elementApplicationMode != mode)
                    {
                        if (element.applicationMode != Mode.Output)
                        {
                            element.rootElement.Visibility = Visibility.Collapsed;
                        }

                        if (enableStateConfig && elementApplicationMode == Mode.GeneralStateConfig)
                        {
                            element.rootElement.Visibility = Visibility.Visible;

                            String quickTitleText = GetModeAsText(mode);
                            if (mode != Mode.Shader)
                            {
                                quickTitleText += " config element";
                            }
                            else
                            {
                                quickTitleText += "s element";
                            }
                            quickTitleText = Char.ToUpper(quickTitleText[0]) + quickTitleText.Substring(1);
                            quickTitle.Text = quickTitleText;
                            if (enableStateConfigAddButton)
                            {
                                addButton.Visibility = Visibility.Visible;
                                addButton.Focus();
                            }
                            else
                            {
                                addButton.Visibility = Visibility.Collapsed;
                            }
                        }

                        DetermineQuickDescriptionVisibilityAndContentByApplicationMode(mode);
                    }
                    else
                    {
                        element.rootElement.Visibility = Visibility.Visible;
                    }
                }

                CleanUpGridRows(cGrid, mode);

                // Set button style for application mode buttons
                if (enableStateConfig)
                {
                    setButtonStyleSelected(GetApplicationModeButtonByApplicationMode(mode));
                }
            }
            catch (System.Exception e)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to set the application mode: " +
                    e.ToString());
            }
        }

        // Note: C# default initializes to zero equivalent.
        protected struct ModeCountStructure
        {
            public UInt32 ShaderCount;
            public UInt32 HitGroupCount;
            public UInt32 LocalRootSignatureCount;
            public UInt32 GlobalRootSignatureCount;
            public UInt32 ShaderPipelineCount;
            public UInt32 InfoCount;
            public UInt32 ApplicationErrorCount;
        }
        protected ModeCountStructure ModeCount = new ModeCountStructure()
        {
            ShaderCount = 0,
            HitGroupCount = 0,
            LocalRootSignatureCount = 0,
            GlobalRootSignatureCount = 0,
            ShaderPipelineCount = 0,
            InfoCount = 0,
        };

        private int GetAddButtonWidthByMode(Mode mode)
        {
            try
            {
                int smallButton = 235;
                int largeButton = 380;
                int width = 0;
                switch (mode)
                {
                    case Mode.GeneralStateConfig:
                        width = smallButton;
                        break;
                    case Mode.Shader:
                        width = smallButton;
                        break;
                    case Mode.HitGroup:
                        width = smallButton;
                        break;
                    case Mode.LocalRootSignature:
                        width = largeButton;
                        break;
                    case Mode.GlobalRootSignature:
                        width = largeButton;
                        break;
                    case Mode.RaytracingPipeline:
                        width = largeButton;
                        break;
                    case Mode.ShaderPipeline:
                        width = largeButton;
                        break;
                    case Mode.ApplicationError:
                        postErrorMessage("A call to " + nameof(GetAddButtonWidthByMode) +
                            " has been initiated by an invalid mode: " + mode);
                        break;
                }

                return width;
            }
            catch (System.Exception e)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to get the 'add' button width by application mode: " +
                    e.ToString());
                return 0;
            }
        }

        private String GetModeAsText(Mode mode)
        {
            try
            {
                String modeAsText = "";
                switch (mode)
                {
                    case Mode.Welcome:
                        modeAsText = "welcome";
                        break;
                    case Mode.GeneralStateConfig:
                        modeAsText = "general";
                        break;
                    case Mode.Shader:
                        modeAsText = "shader";
                        break;
                    case Mode.HitGroup:
                        modeAsText = "hit group";
                        break;
                    case Mode.LocalRootSignature:
                        modeAsText = "local root signature";
                        break;
                    case Mode.GlobalRootSignature:
                        modeAsText = "global root signature";
                        break;
                    case Mode.RaytracingPipeline:
                        modeAsText = "pipeline";
                        break;
                    case Mode.ShaderPipeline:
                        modeAsText = "shader";
                        break;
                }

                return modeAsText;
            }
            catch (System.Exception e)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to get the application mode as text: " +
                    e.ToString());
                return null;
            }
        }

        public bool VerifyModeCount(Mode mode)
        {
            bool retVal = true;
            uint reportedModeCount= GetModeCount(mode);
            uint empiricalModeCount = 0;
            foreach (MacroElement macroElement in Master_MacroElementList)
            {
                if (macroElement.applicationMode == mode)
                {
                    empiricalModeCount++;
                }
            }

            if (empiricalModeCount != reportedModeCount)
            {
                postErrorMessage("A call to " + nameof(VerifyModeCount)
                    + "has discovered errors in mode count calculation for mode: " + mode);
                retVal = false;
            }

            return retVal;
        }

        private UInt32 GetModeCount(Mode mode)
        {
            try
            {
                UInt32 retVal = 0;
                if (mode == Mode.Welcome ||
                    mode == Mode.GeneralStateConfig ||
                    mode == Mode.RaytracingPipeline ||
                    mode == Mode.ApplicationError)
                {
                    return retVal;
                    //postWarningMessage("A call to " + nameof(GetModeCount) + " received an invalid Mode: " + mode);
                }
                else
                {
                    switch (mode)
                    {
                        case Mode.Shader:
                            retVal = ModeCount.ShaderCount;
                            break;
                        case Mode.HitGroup:
                            retVal = ModeCount.HitGroupCount;
                            break;
                        case Mode.LocalRootSignature:
                            retVal = ModeCount.LocalRootSignatureCount;
                            break;
                        case Mode.GlobalRootSignature:
                            retVal = ModeCount.GlobalRootSignatureCount;
                            break;
                        case Mode.ShaderPipeline:
                            retVal = ModeCount.ShaderPipelineCount;
                            break;
                    }
                }

                return retVal;
            }
            catch (System.Exception e)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to get the mode count: " +
                    e.ToString());
                return 0;
            }
        }

        private UInt32 DecrementModeCount(Mode mode)
        {
            try
            {
                UInt32 retVal = 0;
                if (mode == Mode.Welcome ||
                    mode == Mode.GeneralStateConfig ||
                    mode == Mode.RaytracingPipeline ||
                    mode == Mode.ApplicationError)
                {
                    postWarningMessage("A call to " + nameof(DecrementModeCount) + " received an invalid Mode: " + mode);
                }
                else
                {
                    switch (mode)
                    {
                        case Mode.Shader:
                            retVal = --ModeCount.ShaderCount;
                            break;
                        case Mode.HitGroup:
                            retVal = --ModeCount.HitGroupCount;
                            break;
                        case Mode.LocalRootSignature:
                            retVal = --ModeCount.LocalRootSignatureCount;
                            break;
                        case Mode.GlobalRootSignature:
                            retVal = --ModeCount.GlobalRootSignatureCount;
                            break;
                        case Mode.ShaderPipeline:
                            retVal = --ModeCount.ShaderPipelineCount;
                            break;
                    }
                }

                return retVal;
            }
            catch (System.Exception e)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to decrement the mode count: " +
                    e.ToString());
                return 0;
            }
        }
    }
}
