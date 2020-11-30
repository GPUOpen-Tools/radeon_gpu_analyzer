using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;


namespace DXR_State_Editor
{
    public partial class MainWindow : Window
    {
        public class MainGridKeyComboHandler
        {
            private List<Mode> modeOrder = new List<Mode>()
            {
                Mode.Shader,
                Mode.HitGroup,
                Mode.LocalRootSignature,
                Mode.GlobalRootSignature,
                Mode.RaytracingPipeline,
                Mode.ShaderPipeline
            };

            private int CurrentModeIndex = -1;

            public Mode GetCurrentModeIndexAsApplicationMode()
            {
                return modeOrder[CurrentModeIndex];
            }

            public void SetCurrentModeIndexByApplicationMode(Mode mode)
            {
                if (mode == Mode.GeneralStateConfig ||
                    mode == Mode.ApplicationError ||
                    mode == Mode.Output)
                {
                    return;
                }

                CurrentModeIndex = modeOrder.IndexOf(mode);
            }

            public Mode IncrementApplicationModeByKeyCobmo(Mode currentMode)
            {
                SetCurrentModeIndexByApplicationMode(currentMode);
                CurrentModeIndex++;

                if (CurrentModeIndex > (modeOrder.Count - 1))
                {
                    CurrentModeIndex = 0;
                }

                return GetCurrentModeIndexAsApplicationMode();
            }

            public Mode DecrementApplicationModeByKeyCobmo(Mode currentMode)
            {
                SetCurrentModeIndexByApplicationMode(currentMode);
                CurrentModeIndex--;

                if (CurrentModeIndex < 0)
                {
                    CurrentModeIndex = (modeOrder.Count - 1);
                }

                return GetCurrentModeIndexAsApplicationMode();
            }
        }
        MainGridKeyComboHandler MasterMainGridKeyComboHandler = new MainGridKeyComboHandler();

        private void HandleMainGridKeyCombos(object sender, KeyEventArgs e)
        {
            if ((e.Key == Key.Tab) &&
                (Keyboard.IsKeyDown(Key.LeftCtrl) || Keyboard.IsKeyDown(Key.RightCtrl)))
            {
                // Handle Ctrl+Tab: Move "down" one in application mode by the app mode button order,
                // (Shader, HitGroup, and so on). Cycle back around (ShaderPipeline -> back to Shader).
                if (Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift))
                {
                    SetApplicationMode(MasterMainGridKeyComboHandler.DecrementApplicationModeByKeyCobmo(CurrentApplicationMode));
                    return;
                }
                // Handle Ctrl+Shift+Tab: Move "Up" one in application mode by the reverse app mode button order,
                // (ShaderPipeline, RaytracingPipeline, and so on). Cycle back around (Shgader-> back to ShaderPipeline).
                else
                {
                    SetApplicationMode(MasterMainGridKeyComboHandler.IncrementApplicationModeByKeyCobmo(CurrentApplicationMode));
                    return;
                }
            }

            // Handle Ctrl+N: Create a new element for the current mode.
            if (e.Key == Key.N &&
                (Keyboard.IsKeyDown(Key.LeftCtrl) || Keyboard.IsKeyDown(Key.RightCtrl)))
            {
                CreateMacroElementByApplicationMode(CurrentApplicationMode);
                DetermineQuickDescriptionVisibilityAndContentByApplicationMode(CurrentApplicationMode);
                return;
            }

            // Handle Ctrl+S: Save JSON.
            if (e.Key == Key.S &&
                (Keyboard.IsKeyDown(Key.LeftCtrl) || Keyboard.IsKeyDown(Key.RightCtrl)))
            {
                SaveJSONAsRPSO();
                return;
            }

            // Handle Ctrl+O: Load JSON.
            if (e.Key == Key.O &&
                (Keyboard.IsKeyDown(Key.LeftCtrl) || Keyboard.IsKeyDown(Key.RightCtrl)))
            {
                LoadJSON("Load JSON From File");
                return;
            }
        }

        private void CoupleTargetAndRootElements(MacroElement element)
        {
            FrameworkElement rootElement = element.rootElement;
            Panel targetElement = element.targetElement;

            if (rootElement != null)
            {
                if (element.applicationMode != Mode.Output)
                {
                    rootElement.Visibility = Visibility.Collapsed;
                }
                else
                {
                    rootElement.Visibility = Visibility.Visible;
                }
            }
            else
            {
                postErrorMessage("Macro element " + element + " has an invalid root element.");
                return;
            }

            if (targetElement != null)
            {
                targetElement.Children.Add(rootElement);
            }
            else
            {
                postErrorMessage("Macro element " + element + " has an invalid target element.");
                return;
            }
        }

        private void InitializeGUILayout()
        {
            // Fill out main grid.
            Grid mainGrid = FindName("mainGrid") as Grid;
            if (mainGrid != null)
            {
                // Handle keystrokes
                KeyDown += new KeyEventHandler(HandleMainGridKeyCombos);

                // Context Menu: Reset
                MenuItem resetMenuItem = new MenuItem();
                resetMenuItem.Header = "Reset/clear all data";
                resetMenuItem.Click += HandleResetJSONContextMenu;

                // Context Menu: Copy JSON
                MenuItem copyJSONMenuItem = new MenuItem();
                copyJSONMenuItem.Header = "Copy JSON text";
                copyJSONMenuItem.Click += ClipboardButton_Click;

                // Context Menu: Save JSON As
                MenuItem saveJSONMenuItem = new MenuItem();
                saveJSONMenuItem.Header = "Save JSON text as";
                saveJSONMenuItem.Click += SaveButton_Click;

                ContextMenu mainContextMenu = new ContextMenu();
                mainContextMenu.Items.Add(resetMenuItem);
                mainContextMenu.Items.Add(copyJSONMenuItem);
                mainContextMenu.Items.Add(saveJSONMenuItem);
                mainGrid.ContextMenu = mainContextMenu;

                // Note: Buttons are build in XAML (Left)
                // Add bottom row spacer to button grid.
                Grid buttonGridParent = FindName("buttonGridParent") as Grid;
                if (buttonGridParent == null)
                {
                    postErrorMessage("Unable to locate " + nameof(buttonGridParent));
                    return;
                }
                CreateMacroElementButtonRow(buttonGridParent, new FrameworkElement());

                // Build Content View Grid (Middle)
                Grid viewGrid = new Grid()
                {
                    HorizontalAlignment = HorizontalAlignment.Stretch
                };
                if (WPFAssignAndRegisterName(viewGrid, nameof(viewGrid), null))
                {
                    viewGrid.SetValue(Grid.ColumnProperty, 1);
                    viewGrid.SetValue(Grid.RowProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(viewGrid));
                    return;
                }

                GridSplitter mainGridSplitter = new GridSplitter()
                {
                    Background = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#FF1A1A1A")),
                    HorizontalAlignment = HorizontalAlignment.Right,
                    VerticalAlignment = VerticalAlignment.Stretch,
                    Width = 5,
                };
                Grid.SetColumn(mainGridSplitter, 1);
                Grid.SetRow(mainGridSplitter, 0);

                // Build JSON View Grid (Right)
                Grid jsonViewGrid = new Grid()
                {
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                };
                if (WPFAssignAndRegisterName(jsonViewGrid, nameof(jsonViewGrid), null))
                {
                    jsonViewGrid.SetValue(Grid.ColumnProperty, 2);
                    jsonViewGrid.SetValue(Grid.RowProperty, 0);
                }
                else
                {
                    postErrorMessage("Unable to create " + nameof(jsonViewGrid));
                    return;
                }

                mainGrid.Children.Add(viewGrid);
                mainGrid.Children.Add(jsonViewGrid);

                // Note: Microsoft recommends adding this last, even though it is "out of order".
                mainGrid.Children.Add(mainGridSplitter);

                //
                // Set up default layouts.
                //
                // Note: Order matters; OutputConfig must come first.
                Master_MacroElementList.Clear();
                Master_MacroElementList = new List<MacroElement>()
                {
                    CreateOutputConfig(),
                    CreateWelcomePane(),
                    CreateGeneralStateConfig(),
                    CreateRaytracingPipelineConfig(),
                };

                // Apply defaults, add to local elements list.
                foreach (MacroElement element in Master_MacroElementList)
                {
                    if (element.applicationMode == Mode.ApplicationError)
                    {
                        postErrorMessage("A call to " + nameof(InitializeGUILayout) +
                            " has detected an ApplicationError mode fo MacroElement: " + element);
                        return;
                    }

                    // Defaults
                    CoupleTargetAndRootElements(element);
                }

                // Show Welcome Pane
                SetApplicationMode(Mode.Welcome);
            }
            else
            {
                postWarningMessage("Unable to find the main grid! Cannot continue.");
            }
        }

        readonly List<Mode> Master_DefaultUserLevelElementList = new List<Mode>()
        {
            Mode.ShaderPipeline,
        };

        public void RemoveDefaultUserLevelElements()
        {
            foreach (Mode mode in Master_DefaultUserLevelElementList)
            {
                if (mode == Mode.Welcome ||
                    mode == Mode.GeneralStateConfig ||
                    mode == Mode.RaytracingPipeline ||
                    mode == Mode.ApplicationError ||
                    mode == Mode.Output)
                {
                    continue;
                }

                uint modeCount = GetModeCount(mode);
                if (modeCount == 0)
                {
                    postErrorMessage("A call to " + nameof(InitializeDefaultUserLevelElements) +
                        " was made without any elements to remove for mode: " + mode);
                    continue;
                }
                else if (modeCount != 1)
                {
                    postErrorMessage("A call to " + nameof(InitializeDefaultUserLevelElements) +
                        " was made while more than one element exists for mode: " + mode);
                    continue;
                }

                SetApplicationMode(mode);
                foreach (MacroElement macroElement in Master_MacroElementList)
                {
                    if (macroElement.applicationMode == mode)
                    {
                        RemoveMacroElement(macroElement);
                        break;
                    }
                }
            }

            // Return to welcome mode
            SetApplicationMode(Mode.Welcome);
        }

        public void InitializeDefaultUserLevelElements()
        {
            foreach (Mode mode in Master_DefaultUserLevelElementList)
            {
                if (GetModeCount(mode) != 0)
                {
                    postErrorMessage("A call to " + nameof(InitializeDefaultUserLevelElements) +
                        " was unable to verify a zero mode count for mode: " + mode);
                    continue;
                }

                SetApplicationMode(mode);
                CreateMacroElementByApplicationMode(mode);
            }

            // Return to welcome mode
            SetApplicationMode(Mode.Welcome);
        }

        void DXR_Closing(object sender, CancelEventArgs e)
        {
            if (QueryJsonForNonDefaultAssociations() == true)
            {
                string msg = "All unsaved state will be lost. Close anyway?";
                MessageBoxResult result =
                  MessageBox.Show(
                    msg,
                    "Unsaved state detected",
                    MessageBoxButton.YesNo,
                    MessageBoxImage.Warning);
                if (result == MessageBoxResult.No)
                {
                    // If user doesn't want to close, cancel closure
                    e.Cancel = true;
                }
            }
        }

        public MainWindow()
        {
            try
            {
                InitializeComponent();
                InitializeGUILayout();
                InitializeDefaultUserLevelElements();

                // increase tooltip display time
                ToolTipService.ShowDurationProperty.OverrideMetadata(typeof(DependencyObject),
                    new FrameworkPropertyMetadata(Int32.MaxValue));

                // Populate illegal char array
                invalidFilePathChars.AddRange(Path.GetInvalidPathChars());
                invalidFileNameChars.AddRange(Path.GetInvalidFileNameChars());
            }
            catch (System.Exception e)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to initialize the main window: " +
                    e.ToString());
            }
        }
    }
}