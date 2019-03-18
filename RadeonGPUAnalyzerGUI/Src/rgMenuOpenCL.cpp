// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QWidget>
#include <QBoxLayout>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAddCreateMenuItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItemOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuTitlebar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgHandleTabFocusEventFilter.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <QtCommon/Scaling/ScalingManager.h>

static const char* s_BUTTON_FOCUS_IN_STYLESHEET_OPENCL = "QPushButton { background: rgb(253,255,174); border: 2px inset rgb(18, 152, 0); }";

rgMenuOpenCL::rgMenuOpenCL(QWidget* pParent)
    : rgMenu(pParent)
{
}

void rgMenuOpenCL::InitializeDefaultMenuItems(const std::shared_ptr<rgProjectClone>)
{
    // Insert the "Build Settings" item into the top of the menu.
    m_pBuildSettingsMenuItem = new rgMenuBuildSettingsItem();
    m_pLayout->insertWidget(0, m_pBuildSettingsMenuItem);

    // Insert the "Add/Create" menu item to the top of the menu.
    m_pDefaultMenuItem = new rgAddCreateMenuItem(this);
    m_pLayout->insertWidget(0, m_pDefaultMenuItem);

    ConnectDefaultItemSignals();

    ConnectButtonSignals();

    // Make the menu as wide as the items.
    const int height = m_pDefaultMenuItem->height();
    this->resize(m_pDefaultMenuItem->width(), 2 * (height));

    // Register the buttons with the scaling manager.
    ScalingManager::Get().RegisterObject(m_pBuildSettingsMenuItem);
    ScalingManager::Get().RegisterObject(m_pDefaultMenuItem);
}

void rgMenuOpenCL::ConnectDefaultItemSignals()
{
    // Handler invoked when the "Add File" button is clicked within an API item.
    bool isConnected = connect(m_pDefaultMenuItem->GetAddButton(), &QPushButton::clicked, this, &rgMenu::OpenFileButtonClicked);
    assert(isConnected);

    // Handler invoked when the "Create File" button is clicked within an API item.
    isConnected = connect(m_pDefaultMenuItem->GetCreateButton(), &QPushButton::clicked, this, &rgMenu::CreateFileButtonClicked);
    assert(isConnected);
}

void rgMenuOpenCL::ConnectButtonSignals()
{
    // Handler invoked when the "Build settings" button is clicked.
    bool isConnected = connect(m_pBuildSettingsMenuItem->GetBuildSettingsButton(), &QPushButton::clicked, this, &rgMenu::BuildSettingsButtonClicked);
    assert(isConnected);

    isConnected = connect(m_pBuildSettingsMenuItem->GetBuildSettingsButton(), &QPushButton::clicked, this, &rgMenu::HandleBuildSettingsButtonClicked);
    assert(isConnected);
}

void rgMenuOpenCL::ConnectMenuFileItemSignals(rgMenuFileItem* pMenuItem)
{
    rgMenuFileItemOpenCL* pOpenCLItem = static_cast<rgMenuFileItemOpenCL*>(pMenuItem);

    assert(pOpenCLItem != nullptr);
    if (pOpenCLItem != nullptr)
    {
        // Connect the close button for the file's menu item.
        bool isConnected = connect(pOpenCLItem, &rgMenuFileItemOpenCL::MenuItemCloseButtonClicked, this, &rgMenu::MenuItemCloseButtonClicked);
        assert(isConnected);

        // Connect the file menu item selection handler for each new item.
        isConnected =  connect(pOpenCLItem, &rgMenuFileItemOpenCL::MenuItemSelected, this, &rgMenu::MenuItemClicked);
        assert(isConnected);

        // Connect the file menu item selection handler to update build settings button.
        isConnected = connect(pMenuItem, &rgMenuFileItemOpenCL::MenuItemSelected, this, &rgMenu::HandleActivateItemAction);
        assert(isConnected);

        // Connect the file menu item rename handler for each new item.
        isConnected =  connect(pOpenCLItem, &rgMenuFileItemOpenCL::FileRenamed, this, &rgMenu::HandleRenamedFile);
        assert(isConnected);

        // Connect the file menu item's entry point changed handler.
        isConnected =  connect(pOpenCLItem, &rgMenuFileItemOpenCL::SelectedEntrypointChanged, this, &rgMenuOpenCL::SelectedEntrypointChanged);
        assert(isConnected);
    }
}

bool rgMenuOpenCL::AddItem(const std::string& fullPath, bool isNewFileItem)
{
    bool wasAdded = false;

    // Extract the file name from the full file path.
    std::string fileName;
    bool isOk = rgUtils::ExtractFileName(fullPath, fileName);
    assert(isOk);
    if (isOk)
    {
        // If a file with that name hasn't been added.
        if (m_fullFilepathToMenuItem.find(fullPath) == m_fullFilepathToMenuItem.end())
        {
            // Create the menu item widget and add it to the UI and to the list.
            rgMenuFileItemOpenCL* pNewMenuItem = new rgMenuFileItemOpenCL(fullPath, this);
            m_menuItems.push_back(pNewMenuItem);

            // Connect to this file item to change the stylesheet when the build is successful.
            bool isConnected = connect(this, &rgMenuOpenCL::ProjectBuildSuccess, pNewMenuItem, &rgMenuFileItemOpenCL::HandleProjectBuildSuccess);
            assert(isConnected);

            // Emit a signal that indicates that the number of items in the menu change,
            // and specify that the menu is not empty.
            emit FileMenuItemCountChanged(false);

            // Add the file name and its full path to the map.
            m_fullFilepathToMenuItem[fullPath] = pNewMenuItem;

            // Insert the item just above the default "CL" menu item.
            const size_t index = (!m_menuItems.empty()) ? (m_menuItems.size() - 1) : 0;
            m_pLayout->insertWidget(static_cast<const int>(index), pNewMenuItem, Qt::AlignTop);

            // Register the object with the scaling manager.
            ScalingManager::Get().RegisterObject(pNewMenuItem);

            // Connect signals for the new file menu item.
            ConnectMenuFileItemSignals(pNewMenuItem);

            // Select the file that was just opened.
            HandleSelectedFileChanged(pNewMenuItem);

            if (isNewFileItem)
            {
                // If a file was newly-created (as opposed to being opened), display the
                // item's renaming control immediately so the user can choose a new filename.
                pNewMenuItem->ShowRenameControls(true);
            }

            wasAdded = true;
        }
        else
        {
            // Report the error.
            std::stringstream msg;
            msg << STR_ERR_CANNOT_ADD_FILE_A << fileName << STR_ERR_CANNOT_ADD_FILE_B;
            rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
        }
    }

    return wasAdded;
}

void rgMenuOpenCL::ClearBuildOutputs()
{
    // Step through each file item in menu and clear the entry point list.
    for (rgMenuFileItem* pFileItem : m_menuItems)
    {
        rgMenuFileItemOpenCL* pFileItemOpenCL = static_cast<rgMenuFileItemOpenCL*>(pFileItem);
        assert(pFileItemOpenCL != nullptr);
        if (pFileItemOpenCL != nullptr)
        {
            pFileItemOpenCL->ClearEntrypointsList();
        }
    }
}

bool rgMenuOpenCL::GetIsShowEntrypointListEnabled() const
{
    return m_isShowEntrypointListEnabled;
}

bool rgMenuOpenCL::OffsetCurrentFileEntrypoint(int offset)
{
    bool ret = false;

    // The offset should be a positive or negative integer. If it's not, something is wrong.
    assert(offset != 0);

    // If focus index is in the range of the menu file items, select the appropriate file item.
    bool isFocusInFileItemRange = m_focusIndex < m_menuItems.size();
    assert(isFocusInFileItemRange);
    if (isFocusInFileItemRange)
    {
        rgMenuFileItem* pFileItem = m_menuItems[m_focusIndex];

        rgMenuFileItemOpenCL* pOpenCLItem = static_cast<rgMenuFileItemOpenCL*>(pFileItem);

        assert(pOpenCLItem != nullptr);
        if (pOpenCLItem != nullptr)
        {
            // Get the list of entry point names in the current file.
            std::vector<std::string> entrypointNames;
            pOpenCLItem->GetEntrypointNames(entrypointNames);

            if (!entrypointNames.empty())
            {
                // Get the name of the currently selected entrypoint.
                std::string selectedEntrypointName;
                bool isEntrypointSelected = pOpenCLItem->GetSelectedEntrypointName(selectedEntrypointName);
                if (isEntrypointSelected && !selectedEntrypointName.empty())
                {
                    // Compute the index of the currently selected entrypoint.
                    auto currentIndexIter = std::find(entrypointNames.begin(), entrypointNames.end(), selectedEntrypointName);
                    int selectedIndex = currentIndexIter - entrypointNames.begin();

                    // Offset the index, and check that it's still a valid index in the name list.
                    selectedIndex += offset;
                    if (selectedIndex >= 0 && selectedIndex < entrypointNames.size())
                    {
                        // Emit a signal indicating that the selected entry point has changed.
                        auto nextEntrypointIter = entrypointNames.begin() + selectedIndex;
                        emit SelectedEntrypointChanged(pOpenCLItem->GetFilename(), *nextEntrypointIter);
                        ret = true;
                    }
                }
            }
        }
    }

    return ret;
}

void rgMenuOpenCL::SetIsShowEntrypointListEnabled(bool isEnabled)
{
    m_isShowEntrypointListEnabled = isEnabled;

    // Disable the ability to expand file items' entry point list.
    if (!isEnabled && m_pSelectedFileItem != nullptr)
    {
        rgMenuFileItemOpenCL* pMenuItemOpenCL = static_cast<rgMenuFileItemOpenCL*>(m_pSelectedFileItem);
        assert(pMenuItemOpenCL != nullptr);
        if (pMenuItemOpenCL != nullptr)
        {
            pMenuItemOpenCL->ShowEntrypointsList(false);
        }
    }
}

void rgMenuOpenCL::UpdateBuildOutput(const rgBuildOutputsMap& buildOutputs)
{
    std::string gpuWithOutputs;
    std::shared_ptr<rgCliBuildOutput> pGpuOutputs = nullptr;
    bool isOutputValid = rgUtils::GetFirstValidOutputGpu(buildOutputs, gpuWithOutputs, pGpuOutputs);
    if (isOutputValid && (pGpuOutputs != nullptr))
    {
        std::shared_ptr<rgCliBuildOutputOpenCL> pGpuOutputsOpenCL =
            std::dynamic_pointer_cast<rgCliBuildOutputOpenCL>(pGpuOutputs);

        assert(pGpuOutputsOpenCL != nullptr);
        if (pGpuOutputsOpenCL != nullptr)
        {
            // Step through each file item in the menu.
            for (rgMenuFileItem* pFileItem : m_menuItems)
            {
                rgMenuFileItemOpenCL* pFileItemOpenCL = static_cast<rgMenuFileItemOpenCL*>(pFileItem);

                // Find the build output with the current item's filename.
                const std::string& itemFilename = pFileItem->GetFilename();
                auto outputIter = pGpuOutputsOpenCL->m_perFileOutput.find(itemFilename);
                if (outputIter != pGpuOutputsOpenCL->m_perFileOutput.end())
                {
                    // Update the menu item with the outputs for the matching input file.
                    rgFileOutputs& fileOutputs = outputIter->second;
                    const std::vector<rgEntryOutput>& entryOutputs = fileOutputs.m_outputs;
                    pFileItemOpenCL->UpdateBuildOutputs(entryOutputs);
                }
            }

            // Does the user have a file selected in the menu?
            rgMenuFileItemOpenCL* pFileItemOpenCL = static_cast<rgMenuFileItemOpenCL*>(m_pSelectedFileItem);
            if (pFileItemOpenCL != nullptr)
            {
                // Auto-expand the list of entrypoints in the selected file item.
                const std::string& selectedFilename = pFileItemOpenCL->GetFilename();
                pFileItemOpenCL->ShowEntrypointsList(true);
            }
        }
    }
}

void rgMenuOpenCL::HandleBuildStarted()
{
    // Call the base class implementation.
    rgMenu::HandleBuildStarted();

    // Don't allow the user to expand the entry point list for file items.
    SetIsShowEntrypointListEnabled(false);

    // While a build is in progress, disable adding/creating files or changing the build settings.
    m_pDefaultMenuItem->GetAddButton()->setEnabled(false);
    m_pDefaultMenuItem->GetCreateButton()->setEnabled(false);
}

void rgMenuOpenCL::HandleBuildEnded()
{
    // Call the base class implementation.
    rgMenu::HandleBuildEnded();

    // Re-enable allowing the user to expand the entry point list for file items.
    SetIsShowEntrypointListEnabled(true);

    // After the build is over, re-enable the menu items.
    m_pDefaultMenuItem->GetAddButton()->setEnabled(true);
    m_pDefaultMenuItem->GetCreateButton()->setEnabled(true);
}

void rgMenuOpenCL::SelectFocusItem(FileMenuActionType actionType)
{
    assert(m_pBuildSettingsMenuItem != nullptr);
    if (m_pBuildSettingsMenuItem != nullptr)
    {
        // Clear button style sheets.
        if (!m_pBuildSettingsMenuItem->IsCurrent())
        {
            m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet("");
        }
        QPushButton* pAddButton = m_pDefaultMenuItem->GetAddButton();
        QPushButton* pCreateButton = m_pDefaultMenuItem->GetCreateButton();
        assert(pAddButton != nullptr);
        assert(pCreateButton != nullptr);
        if (pAddButton != nullptr)
        {
            pAddButton->setStyleSheet("");
        }
        if (pCreateButton != nullptr)
        {
            pCreateButton->setStyleSheet("");
        }

        // If focus index is in the range of the menu file items, select the appropriate file item.
        if (m_focusIndex < m_menuItems.size())
        {
            rgMenuFileItemOpenCL* pItem = static_cast<rgMenuFileItemOpenCL*>(m_menuItems[m_focusIndex]);
            assert(pItem != nullptr);
            if (pItem != nullptr)
            {
                UpdateHighlight(pItem);
            }

            // Update the cursor.
            UpdateCursor(pItem);
        }
        else
        {
            // Deselect OpenCL menu file items.
            for (rgMenuFileItem* pItem : m_menuItems)
            {
                rgMenuFileItemOpenCL* pItemOpenCL = static_cast<rgMenuFileItemOpenCL*>(pItem);
                assert(pItem != nullptr);
                if (pItem != nullptr)
                {
                    pItem->SetHovered(false);
                }
            }

            // If out of range, special case handle the last focus items.
            // Get index excluding file items.
            size_t index = m_focusIndex - m_menuItems.size();

            // Handle special cases for Add, Create and Build settings buttons.
            switch (index)
            {
            case static_cast<size_t>(FileMenuFocusItems::AddButton) :
            {
                if (pAddButton != nullptr)
                {
                    pAddButton->setStyleSheet(s_BUTTON_FOCUS_IN_STYLESHEET_OPENCL);
                }
                break;
            }
            case static_cast<size_t>(FileMenuFocusItems::CreateButton):
            {
                if (pCreateButton != nullptr)
                {
                    pCreateButton->setStyleSheet(s_BUTTON_FOCUS_IN_STYLESHEET_OPENCL);
                }
                break;
            }
            case static_cast<size_t>(FileMenuFocusItems::BuildSettingsButton):
                m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_FOCUS_IN_STYLESHEET_OPENCL);
                break;
            default:
                // Should never get here.
                assert(false);
                break;
            }
        }
    }
}

void rgMenuOpenCL::HandleActivateItemAction()
{
    assert(m_pDefaultMenuItem != nullptr);
    if (m_pDefaultMenuItem != nullptr)
    {
        // If focus index is in the range of the menu file items, select the appropriate file item.
        if (m_focusIndex < m_menuItems.size())
        {
            SelectFile(m_menuItems[m_focusIndex]);

            // Set the build settings button to have focus out style sheets.
            assert(m_pBuildSettingsMenuItem != nullptr);
            if (m_pBuildSettingsMenuItem != nullptr)
            {
                m_pBuildSettingsMenuItem->SetCurrent(false);
                m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_FOCUS_OUT_STYLESHEET);

                // Set the cursors.
                m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setCursor(Qt::PointingHandCursor);
            }
        }
        else
        {
            // Deselect menu file items.
            for (rgMenuFileItem* pItem : m_menuItems)
            {
                rgMenuFileItemOpenCL* pItemOpenCL = static_cast<rgMenuFileItemOpenCL*>(pItem);
                assert(pItemOpenCL != nullptr);
                if (pItemOpenCL != nullptr)
                {
                    pItemOpenCL->SetHovered(false);
                    pItemOpenCL->SetCurrent(false);
                }
            }

            // If out of range, special case handle the last focus items.
            // Get index excluding file items.
            size_t index = m_focusIndex - m_menuItems.size();

            // Handle special cases for Add, Create and Build settings buttons.
            switch (index)
            {
            case static_cast<size_t>(FileMenuFocusItems::AddButton):
            {
                QPushButton * pAddButton = m_pDefaultMenuItem->GetAddButton();
                assert(pAddButton != nullptr);
                if (pAddButton != nullptr)
                {
                    pAddButton->click();
                }
                break;
            }
            case static_cast<size_t>(FileMenuFocusItems::CreateButton):
            {
                QPushButton * pCreateButton = m_pDefaultMenuItem->GetCreateButton();
                assert(pCreateButton != nullptr);
                if (pCreateButton != nullptr)
                {
                    pCreateButton->click();
                }
                break;
            }
            case static_cast<size_t>(FileMenuFocusItems::BuildSettingsButton):
                assert(m_pBuildSettingsMenuItem != nullptr);
                if (m_pBuildSettingsMenuItem != nullptr)
                {
                    m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_FOCUS_IN_STYLESHEET);
                    m_pBuildSettingsMenuItem->GetBuildSettingsButton()->click();
                }
                break;
            default:
                // Should never get here.
                assert(false);
                break;
            }
        }
    }
}

void rgMenuOpenCL::HandleBuildSettingsButtonClicked(bool /*checked*/)
{
    // Set the stylesheet for each file menu item to be not selected (grayed out),
    // along with the mouse cursor to pointing hand cursor.
    for (rgMenuFileItem* pItem : m_menuItems)
    {
        pItem->SetHovered(false);
        pItem->SetCurrent(false);
        pItem->setCursor(Qt::PointingHandCursor);
    }

    // Set the focus index.
    m_focusIndex = m_menuItems.size() + GetButtonCount() - 1;

    // Set the "current" property value to true for build settings button.
    m_pBuildSettingsMenuItem->SetCurrent(true);

    // Set the focus for the build settings button stylesheet.
    m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_FOCUS_IN_STYLESHEET_OPENCL);

    // Set the mouse cursor for the build settings button.
    m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setCursor(Qt::ArrowCursor);
}

void rgMenuOpenCL::HandleSelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName)
{
    // Find the given input file and select the incoming entry point name.
    for (rgMenuFileItem* pFileItem : m_menuItems)
    {
        if (pFileItem->GetFilename().compare(inputFilePath) == 0)
        {
            rgMenuFileItemOpenCL* pFileItemOpenCL = static_cast<rgMenuFileItemOpenCL*>(pFileItem);
            assert(pFileItemOpenCL != nullptr);
            if (pFileItemOpenCL != nullptr)
            {
                // Switch to the given entry point in the file item's entry point list.
                pFileItemOpenCL->SwitchToEntrypointByName(selectedEntrypointName);
            }
        }
    }
}

void rgMenuOpenCL::HandleNextItemAction()
{
    size_t endIndex = m_menuItems.size() + 2;

    // If a file item is currently selected, switch to the next available entrypoint.
    bool nextEntrypointSelected = false;
    if (m_focusIndex < m_menuItems.size())
    {
        nextEntrypointSelected = OffsetCurrentFileEntrypoint(1);
    }

    if (!nextEntrypointSelected)
    {
        // Increment focus index and wrap around.
        m_focusIndex++;
        if (m_focusIndex > endIndex)
        {
            m_focusIndex = 0;
        }

        // Refresh focus.
        SelectFocusItem(FileMenuActionType::ArrowAction);
    }
}

void rgMenuOpenCL::HandlePreviousItemAction()
{
    size_t endIndex = m_menuItems.size() + 2;

    // If a file item is currently selected, switch to the next available entrypoint.
    bool previousEntrypointSelected = false;
    if (m_focusIndex < m_menuItems.size())
    {
        previousEntrypointSelected = OffsetCurrentFileEntrypoint(-1);
    }

    if (!previousEntrypointSelected)
    {
        // Decrement focus index and wrap around.
        if (m_focusIndex != 0)
        {
            m_focusIndex--;
        }
        else
        {
            m_focusIndex = endIndex;
        }

        // Refresh focus.
        SelectFocusItem(FileMenuActionType::ArrowAction);
    }
}

void rgMenuOpenCL::HandleSourceFileAdded()
{
    rgMenuBuildSettingsItem* pBuildSettingsButton = GetBuildSettingsItem();
    assert(pBuildSettingsButton != nullptr);
    if (pBuildSettingsButton != nullptr)
    {
        pBuildSettingsButton->SetCurrent(false);
    }
}

int rgMenuOpenCL::GetButtonCount() const
{
    // There are 3 extra buttons besides the file items:
    // Add existing file
    // Create new file
    // Build settings
    static const int s_EXTRA_OPENCL_BUTTON_COUNT = 3;
    return s_EXTRA_OPENCL_BUTTON_COUNT;
}

void rgMenuOpenCL::HandleSelectedFileChanged(rgMenuFileItem* pSelected)
{
    // Set focus index to newly selected item.
    for (size_t i = 0; i < m_menuItems.size(); i++)
    {
        if (m_menuItems[i] == pSelected)
        {
            m_focusIndex = i;
            m_tabFocusIndex = i;
            break;
        }
    }

    // Display the currently selected file in the source editor.
    DisplayFileInEditor(m_menuItems[m_focusIndex]);
}

void rgMenuOpenCL::SetButtonsNoFocus()
{
    assert(m_pBuildSettingsMenuItem != nullptr);

    // Set the build settings button to have focus out style sheets.
    if (m_pBuildSettingsMenuItem != nullptr)
    {
        QPushButton* pBuildSettingsButton = m_pBuildSettingsMenuItem->GetBuildSettingsButton();
        assert(pBuildSettingsButton != nullptr);
        if (pBuildSettingsButton != nullptr)
        {
            pBuildSettingsButton->setStyleSheet(s_BUTTON_FOCUS_OUT_STYLESHEET);
        }
    }
}

void rgMenuOpenCL::SelectTabFocusItem(bool shiftTabFocus)
{
    // @TODO: Need to add tabbing functionality in OpenCL file menu.
}

void rgMenuOpenCL::HandleTabFocusPressed()
{
    // @TODO: Need to add tabbing functionality in OpenCL file menu.
}

void rgMenuOpenCL::HandleShiftTabFocusPressed()
{
    // @TODO: Need to add tabbing functionality in OpenCL file menu.
}