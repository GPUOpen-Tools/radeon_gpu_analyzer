#pragma once

// C++.
#include <string>
#include <list>
#include <map>
#include <memory>

// Qt.
#include <QString>

// Local.
#include <ui_rgFileMenu.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>

// Forward declarations.
class QWidget;
class QVBoxLayout;
class rgFileMenuItem;
class rgFileMenuTitlebar;
class rgFileMenuBuildSettingsItem;
class rgFileMenuFileItem;
class rgAddCreateMenuItem;
class rgBuildView;
struct rgCliBuildOutput;

typedef std::map<std::string, rgFileMenuFileItem*> StringToFileItemMap;

class rgFileMenu :
    public QFrame
{
    Q_OBJECT

public:
    rgFileMenu(QWidget* pParent);
    ~rgFileMenu() = default;

    // Create all file menu actions.
    void CreateActions();

    // Add an item to the menu.
    void AddItem(const std::string& fullFilename, bool isNewFileItem = false);

    // Offset the currently selected row in the entrypoint list by advancing or regressing the current index by the given offset.
    // This function will return true only when (current index + offset) is a valid index in the entrypoint list.
    bool OffsetCurrentFileEntrypoint(int offset);

    // Clear the file menu's build outputs for each source file in the project.
    void ClearBuildOutputs();

    // Clear the menu of all file items.
    void ClearFiles();

    // Generate a filename that is unique when compared to other files already opened in the file menu.
    void GenerateUniqueFilename(const std::string& initialFilename, std::string& uniqueFilename) const;

    // Get the Build Settings file menu item.
    rgFileMenuBuildSettingsItem* GetBuildSettingsItem() const;

    // Retrieve the file menu item associated with the given source file path.
    rgFileMenuFileItem* GetFileItemFromPath(const std::string& sourceFilePath) const;

    // Gets whether or not showing entrypoint lists within file items is enabled.
    bool GetIsShowEntrypointListEnabled() const;

    // Get the currently selected menu file item.
    rgFileMenuFileItem* GetSelectedFileItem() const;

    // Get the full path to the currently selected file item, or an empty string if there is one.
    std::string GetSelectedFilePath() const;

    // Determine if a file with the given name has already been added to the file menu.
    bool IsFileInMenu(const std::string& filename) const;

    // Remove an item from the menu.
    void RemoveItem(const std::string& fullFilename);

    // Handler invoked when the build settings button is clicked.
    void DeselectFiles();

    // Mark an item as saved or unsaved (denoted by "*" after the filename).
    void SetItemIsSaved(const std::string& fullFilename, bool isSaved);

    // Toggle the visibility of the entrypoint list for file items.
    void SetIsShowEntrypointListEnabled(bool isEnabled);

    // Switch to select the first item in the menu.
    void SelectLastRemainingItem();

    // Update the file menu with the latest build output.
    void UpdateBuildOutput(const rgBuildOutputsMap& buildOutputs);

signals:
    // Signal invoked when an item was added/removed to/from the menu,
    // with a boolean parameter that is set to true if no items are left in the menu.
    void FileMenuItemCountChanged(bool isCountZero);

    // Signal invoked when the selected menu item is changed from the current file to a new file.
    void SelectedFileChanged(const std::string& oldFilename, const std::string& newFilename);

    // Signal invoked when a file item is closed within the file menu.
    void FileClosed(const std::string& filename);

    // Signal invoked when a file has been renamed.
    void FileRenamed(const std::string& oldFilepath, const std::string& newFilepath);

    // Signal invoked when the user presses Tab to switch focus away from the file menu.
    void FocusNextView();

    //  Signal invoked when the build settings button is clicked.
    void BuildSettingsButtonClicked();

    // Add a new file from the default menu item.
    void OpenFileButtonClicked();

    // Create a new file from the default menu item.
    void CreateFileButtonClicked();

    // Signal passthrough invoked when the close button is clicked on a code file item.
    void MenuItemCloseButtonClicked(const std::string fullPath);

    // Signal emitted when the user changes the selected entrypoint index for a given file.
    void SelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // Signal used by FileMenu to make the current Code Editor to scroll to the required line.
    void ScrollCodeEditorToLine(int lineNum);

public slots:
    // Handler invoked when the selected entrypoint has been changed.
    void HandleSelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // Handler for when a build has started.
    void HandleBuildStarted();

    // Handler for when a build has ended.
    void HandleBuildEnded();

    // Handler to switch to the required file/line specified by "filePath" and "lineNum".
    void HandleSwitchToFile(const std::string& filePath, int lineNum);

private slots:
    // Handler invoked when a file item has been renamed.
    void HandleRenamedFile(const std::string& oldFilepath, const std::string& newFilepath);

    // Handler invoked when the user changes the currently selected file item.
    void HandleSelectedFileChanged(rgFileMenuFileItem* pSelected);

    // Handler invoked when the user triggers the next menu item action.
    void HandleNextItemAction();

    // Handler invoked when the user triggers the previous menu item action.
    void HandlePreviousItemAction();

    // Handler invoked when the user triggers the open context menu action.
    void HandleOpenContextMenuAction();

    // Handler invoked when the user presses the activate key (Enter).
    // This is used to handle pressing add/create buttons when in focus.
    void HandleActivateItemAction();

    // Handler invoked when the user triggers the rename selected file action.
    void HandleRenameSelectedFileAction();

    // Handler invoked when the user presses the Tab key to switch focus.
    void HandleTabFocusPressed();

private:
    // Populate the menu with the default items that cannot be removed.
    void InitializeDefaultMenuItems();

    // Connect signals to slots for items in the file menu.
    void ConnectDefaultItemSignals();

    // Connect signals for a newly created menu file item.
    void ConnectMenuFileItemSignals(rgFileMenuFileItem* pMenuItem);

    // Select the given file item.
    void SelectFile(rgFileMenuFileItem* pSelected);

    // Select/focus on the item indicated by m_focusIndex.
    void SelectFocusItem();

    // Update the mouse cursor.
    void UpdateCursor();

    // Index of the item currently being focused on.
    size_t m_focusIndex;

    // File menu actions.
    QAction* m_pNextItemAction = nullptr;
    QAction* m_pPrevItemAction = nullptr;
    QAction* m_pOpenContextMenuAction = nullptr;
    QAction* m_activateItemAction = nullptr;
    QAction* m_renameSelectedFileAction = nullptr;

    // The layout that holds the menu items.
    QVBoxLayout*    m_pLayout = nullptr;

    // The default menu item.
    rgAddCreateMenuItem* m_pDefaultMenuItem = nullptr;

    // The build settings menu item.
    rgFileMenuBuildSettingsItem* m_pBuildSettingsMenuItem = nullptr;

    // The currently selected file item within the file menu.
    rgFileMenuFileItem* m_pSelectedFileItem = nullptr;

    // The file menu items.
    std::vector<rgFileMenuFileItem*> m_menuItems;

    // Maps every file name to its corresponding full path.
    StringToFileItemMap m_fullFilepathToMenuItem;

    // A flag used to determine if the kernel list in file items are able to expand.
    bool m_isShowEntrypointListEnabled = true;

    // The Qt interface design.
    Ui::rgFileMenu ui;
};