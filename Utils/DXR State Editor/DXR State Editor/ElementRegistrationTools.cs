using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;

namespace DXR_State_Editor
{
    public partial class MainWindow : Window
    {
        public String GenerateDynamicRowName(Grid grid, Mode mode, UInt32 nameModifier)
        {
            return grid.Name + "Row" + mode + nameModifier;
        }

        public bool WPFAssignAndRegisterName(object element, String elementName)
        {
            return WPFAssignAndRegisterName(element, elementName, null);
        }

        public bool WPFAssignAndRegisterName(object element, String elementName, List<String> nameList)
        {
            try
            {
                bool wasSuccessful = true;
                String methodName = nameof(WPFAssignAndRegisterName);

                if (String.IsNullOrEmpty(elementName))
                {
                    postWarningMessage("A call to " + methodName + " received a null or empty element name.");
                    wasSuccessful = false;
                }

                if (element == null)
                {
                    postWarningMessage("A call to " + methodName + " received a null FrameworkElement: " + elementName);
                    wasSuccessful = false;
                }

                if (FindName(elementName) != null)
                {
                    postWarningMessage("A call to " + methodName +
                        " received a duplicate register request for FrameworkElement: "
                        + elementName);
                    wasSuccessful = false;
                }

                if (wasSuccessful)
                {
                    // Also assign the name to the object itself, if possible.
                    FrameworkElement frameworkElement = element as FrameworkElement;
                    if (frameworkElement != null)
                    {
                        frameworkElement.Name = elementName;
                    }

                    // Register the name with this class.
                    RegisterName(elementName, element);
                    if (FindName(elementName) == null)
                    {
                        postErrorMessage(methodName + " failed to register FrameworkElement: " + elementName);
                        wasSuccessful = false;
                    }

                    // Add registered name to name List, if present.
                    if (nameList != null)
                    {
                        nameList.Add(elementName);
                    }
                }

                return wasSuccessful;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to assign and register an element name: " +
                    ex.ToString());
                return false;
            }
        }

        public List<String> contentGridRowNames = new List<string>();
        public void CleanUpGridRows(Grid grid, Mode mode)
        {
            try
            {
                if (mode == Mode.Welcome ||
                    mode == Mode.GeneralStateConfig ||
                    mode == Mode.RaytracingPipeline ||
                    mode == Mode.ApplicationError)
                {
                    return;
                }

                if (grid == null)
                {
                    postErrorMessage("A call to " + nameof(CleanUpGridRows) + " received a null Grid reference.");
                    return;
                }

                // Delete all of the grid rows.
                UInt32 numRegisteredRowNames = (UInt32)contentGridRowNames.Count;
                UInt32 numRowsRemoved = 0;
                foreach (String name in contentGridRowNames)
                {
                    RowDefinition rowDef = FindName(name) as RowDefinition;
                    if (rowDef != null && name.Length > 0)
                    {
                        UnregisterName(name);
                        if (grid.RowDefinitions.Remove(rowDef)) numRowsRemoved++;
                        else
                        {
                            postErrorMessage("A call to " + nameof(CleanUpGridRows) +
                                " was unable to unregister macro element with name:" + name);
                        }
                    }
                }
                contentGridRowNames.Clear();
                if (numRegisteredRowNames != numRowsRemoved)
                {
                    postErrorMessage("A call to " + nameof(CleanUpGridRows) +
                        " was unable to reconcile the number of registered and physical rows for mode: " + mode
                        + ". [ registered rows: " + numRegisteredRowNames + ", mode physical rows: " + numRowsRemoved + " ]");
                }

                // Make only as many rows as there are items for the given mode.
                UInt32 modeNumber = GetModeCount(mode);
                for (UInt32 index = 0; index < modeNumber; index++)
                {
                    RowDefinition row = new RowDefinition()
                    {
                        Height = new GridLength(1.0, GridUnitType.Auto),
                    };
                    String rowName = GenerateDynamicRowName(grid, mode, index);
                    if (WPFAssignAndRegisterName(row, rowName))
                    {
                        contentGridRowNames.Add(rowName);
                        grid.RowDefinitions.Add(row);
                    }
                }

                // Ensure that mode items match a correctly numbered row.
                UInt32 rowNumber = 0;
                foreach (MacroElement macroElement in Master_MacroElementList)
                {
                    if (macroElement.applicationMode == mode && ((macroElement.targetElement as Grid) != null))
                    {
                        macroElement.rootElement.SetValue(Grid.RowProperty, Convert.ToInt32(rowNumber));
                        rowNumber++;
                    }
                }

                if ((rowNumber > 0) && (rowNumber != modeNumber))
                {
                    postErrorMessage("A call to " + nameof(CleanUpGridRows) +
                        " was unable to reconcile the number of rows and macro elements of mode: " + mode
                        + ". [ rows: " + rowNumber + ", mode elements: " + modeNumber + " ]");
                }
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to clean up the content grid rows: " +
                    ex.ToString());
            }
        }
    }
}
