#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_H_

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
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_file_item.h"
#include "ui_rg_menu.h"

// Forward declarations.
class QWidget;
class QVBoxLayout;
class RgMenuFileItem;
class RgMenuTitlebar;
class RgMenuBuildSettingsItem;
class RgAddCreateMenuItem;
class RgLinkSourceMenuItem;
class RgBuildView;
struct RgCliBuildOutput;

typedef std::map<std::string, RgMenuFileItem*> StringToFileItemMap;

// Stylesheet for add/create buttons when in focus.
static const char* kStrButtonFocusOutStylesheet = "QPushButton { margin: 1px; background: palette(button);}";
static const char* kStrButtonFocusInStylesheet = "QPushButton { border: 1px solid #6666FF; margin: 1px; background: palette(highlight); }";

// Indices for special case file items.
enum class FileMenuFocusItems
{
    kAddButton,
    kCreateButton,
    kBuildSettingsButton
};

// Indices for tabbing/arrow keys in the Graphics file menu.
enum class FileMenuActionType
{
    kArrowAction,
    kTabAction
};

class RgMenu : public QFrame
{
    Q_OBJECT

public:
    RgMenu(QWidget* parent);
    virtual ~RgMenu() = default;

    // Populate the menu with the default items that cannot be removed.
    virtual void InitializeDefaultMenuItems(const std::shared_ptr<RgProjectClone> project_clone) = 0;

    // Connect signals to slots for items in the file menu.
    virtual void ConnectDefaultItemSignals() = 0;

    // Connect signals to slots for Build settings and Pipeline state buttons.
    virtual void ConnectButtonSignals() = 0;

    // Connect signals for the menu file item.
    virtual void ConnectMenuFileItemSignals(RgMenuFileItem* menu_item) = 0;

    // Deselect all menu items.
    virtual void DeselectItems();

    // Select/focus on the item indicated by focus_index_.
    virtual void SelectFocusItem(FileMenuActionType action_type) = 0;

    // Select/focus on the item indicated by focus_index_ when tab pressed.
    virtual void SelectTabFocusItem(bool shift_tab_focus) = 0;

    // Create all file menu actions.
    void CreateActions();

    // Clear the menu of all file items.
    void ClearFiles();

    // Get the Build Settings file menu item.
    RgMenuBuildSettingsItem* GetBuildSettingsItem() const;

    // Retrieve the file menu item associated with the given source file path.
    RgMenuFileItem* GetFileItemFromPath(const std::string& source_file_path) const;

    // Get the currently selected menu file item.
    RgMenuFileItem* GetSelectedFileItem() const;

    // Get the full path to the currently selected file item, or an empty string if there is one.
    std::string GetSelectedFilePath() const;

    // Get all the file items in the menu.
    std::vector<RgMenuFileItem*> GetAllFileItems() const;

    // Return true if the given file item is the
    // currently selected one, and return false otherwise.
    bool IsCurrentlySelectedFileItem(RgMenuFileItem* file_item) const;

    // Return true if the menu is not currently loaded with any files.
    bool IsEmpty() const;

    // Remove an item from the menu.
    virtual void RemoveItem(const std::string& full_filename);

    // Deselect the currently selected file item.
    void DeselectCurrentFile();

    // Mark an item as saved or unsaved (denoted by "*" after the filename).
    void SetItemIsSaved(const std::string& full_filename, bool is_saved);

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
    bool IsFileInMenu(const std::string& full_path) const;

signals:
    // Signal invoked when an item was added/removed to/from the menu,
    // with a boolean parameter that is set to true if no items are left in the menu.
    void FileMenuItemCountChanged(bool is_count_zero);

    // Signal invoked when the selected menu item is changed from the current file to a new file.
    void SelectedFileChanged(const std::string& old_filename, const std::string& new_file_name);

    // Signal invoked when a file item is closed within the file menu.
    void FileClosed(const std::string& filename);

    // Signal invoked when a file has been renamed.
    void FileRenamed(const std::string& old_file_path, const std::string& new_file_path);

    // Signal invoked when the user presses Tab to switch focus away from the file menu.
    void FocusNextView();

    //  Signal invoked when the build settings button is clicked.
    void BuildSettingsButtonClicked();

    // Add a new file from the default menu item.
    void OpenFileButtonClicked();

    // Create a new file from the default menu item.
    void CreateFileButtonClicked();

    // A menu item has been clicked on.
    void MenuItemClicked(RgMenuFileItem* item);

    // Signal passthrough invoked when the close button is clicked on a code file item.
    void MenuItemCloseButtonClicked(const std::string& full_path);

    // Signal used by FileMenu to make the current Code Editor to scroll to the required line.
    void ScrollCodeEditorToLine(int line_num);

    // A signal emitted when the source code editor gets the focus.
    void FileMenuFocusInEvent();

    // A signal emitted when the user clicks graphics file menu.
    void MenuClicked();

protected:
    // Retrieve the number of extra buttons that get added to the menu after the file items.
    virtual int GetButtonCount() const = 0;

    // The overridden mousePressEvent.
    void mousePressEvent(QMouseEvent* event) override;

    // Select the given file item.
    void SelectFile(RgMenuFileItem* selected);

    // Display the selected file in the source editor.
    void DisplayFileInEditor(RgMenuFileItem* selected);

    // Update the mouse cursor.
    void UpdateCursor(RgMenuFileItem* item);

    // Update the file item highlight.
    void UpdateHighlight(RgMenuFileItem* selected);

    // Clear file menu highlight for all items.
    void ClearFileMenuHighlight();

    // Clear file menu items.
    void ClearFileMenuItemSelection();

    // Update the current item setting.
    void UpdateCurrentItem(RgMenuFileItem* item);

    // Maps every full path to corresponding file menu item.
    StringToFileItemMap full_file_path_to_menu_item_;

    // The file menu items.
    std::vector<RgMenuFileItem*> menu_items_;

    // The layout that holds the menu items.
    QVBoxLayout*  layout_ = nullptr;

    // The build settings menu item (a file menu for any API should have a Build Settings item).
    RgMenuBuildSettingsItem* build_settings_menu_item_ = nullptr;

    // Index of the item currently being focused on.
    size_t focus_index_ = 0;

    // Index of the item currently being focused on using tab.
    size_t tab_focus_index_ = 0;

    // File menu actions.
    QAction* next_item_action_ = nullptr;
    QAction* prev_item_action_ = nullptr;
    QAction* open_context_menu_action_ = nullptr;
    QAction* activate_item_action_ = nullptr;
    QAction* rename_selected_file_action_ = nullptr;
    QAction* tab_key_action_ = nullptr;
    QAction* shift_tab_key_action_ = nullptr;

    // The currently selected file item within the file menu.
    RgMenuFileItem* selected_file_item_ = nullptr;

    // Last selected menu item.
    RgMenuFileItem* last_selected_item_ = nullptr;

    // The generated view object.
    Ui::RgMenu ui_;

public slots:
    // Handler invoked when the user presses the activate key (Enter).
    // This is used to handle pressing add/create buttons when in focus.
    virtual void HandleActivateItemAction() = 0;

    // Handler for when a build has started.
    virtual void HandleBuildStarted();

    // Handler for when a build has ended.
    virtual void HandleBuildEnded();

    // Handler to switch to the required file/line specified by "file_path" and "lineNum".
    void HandleSwitchToFile(const std::string& file_path, int line_num);

    // Handler invoked when a file item has been renamed.
    void HandleRenamedFile(const std::string& old_file_path, const std::string& new_file_path);

    // Handler invoked when the user changes the currently selected file item.
    virtual void HandleSelectedFileChanged(RgMenuFileItem* selected) = 0;

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
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_H_
