#pragma once

// C++.
#include <string>
#include <list>
#include <map>
#include <memory>

// Qt.
#include <QString>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenu.h>

// Forward declarations.
class QWidget;
class QVBoxLayout;
class rgMenuItem;
class rgMenuTitlebar;
class rgMenuBuildSettingsItem;
class rgMenuFileItemOpenCL;
class rgAddCreateMenuItem;
class rgBuildView;
struct rgCliBuildOutput;

class rgMenuOpenCL : public rgMenu
{
    Q_OBJECT

public:
    rgMenuOpenCL(QWidget* pParent);
    virtual ~rgMenuOpenCL() = default;

    // Initialize the view's default menu items.
    virtual void InitializeDefaultMenuItems(const std::shared_ptr<rgProjectClone> pProjectClone) override;

    // Connect the signals for the view's default menu items.
    virtual void ConnectDefaultItemSignals() override;

    // Connect signals for the menu file item.
    virtual void ConnectMenuFileItemSignals(rgMenuFileItem* pMenuItem) override;

    // Add an item to the menu.
    bool AddItem(const std::string& fullPath, bool isNewFileItem);

    // Clear the file menu's build outputs for each source file in the project.
    void ClearBuildOutputs();

    // Gets whether or not showing entry point lists within file items is enabled.
    bool GetIsShowEntrypointListEnabled() const;

    // Offset the currently selected row in the entry point list by advancing or regressing the current index by the given offset.
    // This function will return true only when (current index + offset) is a valid index in the entry point list.
    bool OffsetCurrentFileEntrypoint(int offset);

    // Toggle the visibility of the entry point list for file items.
    void SetIsShowEntrypointListEnabled(bool isEnabled);

    // Update the file menu with the latest build output.
    void UpdateBuildOutput(const rgBuildOutputsMap& buildOutputs);

    // Set the build settings button to have no focus.
    virtual void SetButtonsNoFocus() override;

signals:
    // Signal emitted when the user changes the selected entry point index for a given file.
    void SelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // Signal emitted when the project was built successfully.
    void ProjectBuildSuccess();

public slots:
    // Handler for when a build has started.
    virtual void HandleBuildStarted() override;

    // Handler for when a build has ended.
    virtual void HandleBuildEnded() override;

    // Select/focus on the item indicated by m_focusIndex.
    virtual void SelectFocusItem(FileMenuActionType actionType) override;

    // Select/focus on the item indicated by m_focusIndex when tab pressed.
    virtual void SelectTabFocusItem(bool shiftTabFocus) override;

    // Handler invoked when the user presses the activate key (Enter).
    // This is used to handle pressing add/create buttons when in focus.
    virtual void HandleActivateItemAction() override;

    // Handler invoked when the user clicks on the build settings button.
    virtual void HandleBuildSettingsButtonClicked(bool checked) override;

    // Handler invoked when the selected entry point has been changed.
    void HandleSelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // Handler invoked when the user presses the Tab key to switch focus.
    virtual void HandleTabFocusPressed() override;

    // Handler invoked when the user presses the Shift+Tab keys to switch focus.
    virtual void HandleShiftTabFocusPressed() override;

    // Handler invoked when the user adds a new file so the buttons get reset.
    void HandleSourceFileAdded();

    // Handler invoked when the user changes the currently selected file item.
    virtual void HandleSelectedFileChanged(rgMenuFileItem* pSelected) override;

protected slots:
    // Handler invoked when the user triggers the next menu item action.
    virtual void HandleNextItemAction() override;

    // Handler invoked when the user triggers the previous menu item action.
    virtual void HandlePreviousItemAction();

protected:
    // Retrieve the number of extra buttons that get added to the menu after the file items.
    virtual int GetButtonCount() const override;

private:
    // The default menu item.
    rgAddCreateMenuItem* m_pAddCreateMenuItem = nullptr;

    // A flag used to determine if the kernel list in file items are able to expand.
    bool m_isShowEntrypointListEnabled = true;

    // Connect signals for build settings and Pipeline state buttons.
    void ConnectButtonSignals();
};