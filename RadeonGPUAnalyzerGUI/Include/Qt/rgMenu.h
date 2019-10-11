#pragma once

// C++.
#include <string>
#include <list>
#include <map>
#include <memory>

// Qt.
#include <QString>
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QFrame>
#include <QHeaderView>
#include <QScrollArea>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItem.h>
#include <ui_rgMenu.h>

// Forward declarations.
class QWidget;
class QVBoxLayout;
class rgMenuFileItem;
class rgMenuTitlebar;
class rgMenuBuildSettingsItem;
class rgAddCreateMenuItem;
class rgBuildView;
struct rgCliBuildOutput;

typedef std::map<std::string, rgMenuFileItem*> StringToFileItemMap;

// Stylesheet for add/create buttons when in focus.
static const char* s_BUTTON_FOCUS_OUT_STYLESHEET = "QPushButton { margin: 1px; background: rgb(214, 214, 214);}";
static const char* s_BUTTON_FOCUS_IN_STYLESHEET = "QPushButton { border: 1px solid #6666FF; margin: 1px; background: rgb(253,255,174);}";

// Indices for special case file items.
enum class FileMenuFocusItems
{
    AddButton,
    CreateButton,
    BuildSettingsButton
};

// Indices for tabbing/arrow keys in the Graphics file menu.
enum class FileMenuActionType
{
    ArrowAction,
    TabAction
};

class rgMenu : public QFrame
{
    Q_OBJECT

public:
    rgMenu(QWidget* pParent);
    virtual ~rgMenu() = default;

    // Populate the menu with the default items that cannot be removed.
    virtual void InitializeDefaultMenuItems(const std::shared_ptr<rgProjectClone> pProjectClone) = 0;

    // Connect signals to slots for items in the file menu.
    virtual void ConnectDefaultItemSignals() = 0;

    // Connect signals to slots for Build settings and Pipeline state buttons.
    virtual void ConnectButtonSignals() = 0;

    // Connect signals for the menu file item.
    virtual void ConnectMenuFileItemSignals(rgMenuFileItem* pMenuItem) = 0;

    // Deselect all menu items.
    virtual void DeselectItems();

    // Select/focus on the item indicated by m_focusIndex.
    virtual void SelectFocusItem(FileMenuActionType actionType) = 0;

    // Select/focus on the item indicated by m_focusIndex when tab pressed.
    virtual void SelectTabFocusItem(bool shiftTabFocus) = 0;

    // Create all file menu actions.
    void CreateActions();

    // Clear the menu of all file items.
    void ClearFiles();

    // Get the Build Settings file menu item.
    rgMenuBuildSettingsItem* GetBuildSettingsItem() const;

    // Retrieve the file menu item associated with the given source file path.
    rgMenuFileItem* GetFileItemFromPath(const std::string& sourceFilePath) const;

    // Get the currently selected menu file item.
    rgMenuFileItem* GetSelectedFileItem() const;

    // Get the full path to the currently selected file item, or an empty string if there is one.
    std::string GetSelectedFilePath() const;

    // Get all the file items in the menu.
    std::vector<rgMenuFileItem*> GetAllFileItems() const;

    // Return true if the given file item is the
    // currently selected one, and return false otherwise.
    bool IsCurrentlySelectedFileItem(rgMenuFileItem* pFileItem) const;

    // Return true if the menu is not currently loaded with any files.
    bool IsEmpty() const;

    // Remove an item from the menu.
    void RemoveItem(const std::string& fullFilename);

    // Deselect the currently selected file item.
    void DeselectCurrentFile();

    // Mark an item as saved or unsaved (denoted by "*" after the filename).
    void SetItemIsSaved(const std::string& fullFilename, bool isSaved);

    // Switch to select the first item in the menu.
    void SelectFirstItem();

    // Switch to select the last available item in the menu.
    void SelectLastRemainingItem();

    // Set buttons to have no focus.
    virtual void SetButtonsNoFocus() = 0;

    // Switch to last selected item.
    void SwitchToLastSelectedItem();

    // Returns true if the an item representing the given
    // full path is already in the menu, and false otherwise.
    bool IsFileInMenu(const std::string& fullPath) const;

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

    // A menu item has been clicked on.
    void MenuItemClicked(rgMenuFileItem* pItem);

    // Signal passthrough invoked when the close button is clicked on a code file item.
    void MenuItemCloseButtonClicked(const std::string& fullPath);

    // Signal used by FileMenu to make the current Code Editor to scroll to the required line.
    void ScrollCodeEditorToLine(int lineNum);

    // A signal emitted when the source code editor gets the focus.
    void FileMenuFocusInEvent();

    // A signal emitted when the user clicks graphics file menu.
    void MenuClicked();

protected:
    // Retrieve the number of extra buttons that get added to the menu after the file items.
    virtual int GetButtonCount() const = 0;

    // The overridden mousePressEvent.
    void mousePressEvent(QMouseEvent* pEvent) override;

    // Select the given file item.
    void SelectFile(rgMenuFileItem* pSelected);

    // Display the selected file in the source editor.
    void DisplayFileInEditor(rgMenuFileItem* pSelected);

    // Update the mouse cursor.
    void UpdateCursor(rgMenuFileItem* pItem);

    // Update the file item highlight.
    void UpdateHighlight(rgMenuFileItem* pSelected);

    // Clear file menu highlight for all items.
    void ClearFileMenuHighlight();

    // Clear file menu items.
    void ClearFileMenuItemSelection();

    // Update the current item setting.
    void UpdateCurrentItem(rgMenuFileItem* pItem);

    // Maps every full path to corresponding file menu item.
    StringToFileItemMap m_fullFilepathToMenuItem;

    // The file menu items.
    std::vector<rgMenuFileItem*> m_menuItems;

    // The layout that holds the menu items.
    QVBoxLayout*  m_pLayout = nullptr;

    // The build settings menu item (a file menu for any API should have a Build Settings item).
    rgMenuBuildSettingsItem* m_pBuildSettingsMenuItem = nullptr;

    // Index of the item currently being focused on.
    size_t m_focusIndex = 0;

    // Index of the item currently being focused on using tab.
    size_t m_tabFocusIndex = 0;

    // File menu actions.
    QAction* m_pNextItemAction = nullptr;
    QAction* m_pPrevItemAction = nullptr;
    QAction* m_pOpenContextMenuAction = nullptr;
    QAction* m_pActivateItemAction = nullptr;
    QAction* m_pRenameSelectedFileAction = nullptr;
    QAction* m_pTabKeyAction = nullptr;
    QAction* m_pShiftTabKeyAction = nullptr;

    // The currently selected file item within the file menu.
    rgMenuFileItem* m_pSelectedFileItem = nullptr;

    // Last selected menu item.
    rgMenuFileItem* m_pLastSelectedItem = nullptr;

    // The generated view object.
    Ui::rgMenu ui;

public slots:
    // Handler invoked when the user presses the activate key (Enter).
    // This is used to handle pressing add/create buttons when in focus.
    virtual void HandleActivateItemAction() = 0;

    // Handler for when a build has started.
    virtual void HandleBuildStarted();

    // Handler for when a build has ended.
    virtual void HandleBuildEnded();

    // Handler to switch to the required file/line specified by "filePath" and "lineNum".
    void HandleSwitchToFile(const std::string& filePath, int lineNum);

    // Handler invoked when a file item has been renamed.
    void HandleRenamedFile(const std::string& oldFilepath, const std::string& newFilepath);

    // Handler invoked when the user changes the currently selected file item.
    virtual void HandleSelectedFileChanged(rgMenuFileItem* pSelected) = 0;

    // Handler invoked when the user clicks on the build settings button.
    virtual void HandleBuildSettingsButtonClicked(bool /* checked */) = 0;

    // Handler invoked when the user presses the Tab key to switch focus.
    virtual void HandleTabFocusPressed() = 0;

    // Handler invoked when the user presses the Shift+Tab keys to switch focus.
    virtual void HandleShiftTabFocusPressed() = 0;

protected slots:
    // Handler invoked when the user triggers the next menu item action.
    virtual void HandleNextItemAction();

    // Handler invoked when the user triggers the previous menu item action.
    virtual void HandlePreviousItemAction();

private slots:
    // Handler invoked when the user triggers the open context menu action.
    void HandleOpenContextMenuAction();

    // Handler invoked when the user triggers the rename selected file action.
    void HandleRenameSelectedFileAction();
};
