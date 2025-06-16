//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Item widget in RGA Build view's File Menu.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_FILE_ITEM_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_FILE_ITEM_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_item.h"

// Forward declarations.
class QAction;
class QLabel;
class QLineEdit;
class QMenu;

class RgMenuFileItem : public RgMenuItem
{
    Q_OBJECT

public:
    RgMenuFileItem(const std::string& full_file_path, RgMenu* parent = nullptr);
    virtual ~RgMenuFileItem() = default;

    // Alter the visual style of the item if it is hovered or not.
    virtual void SetHovered(bool is_hovered) = 0;

    // Alter the visual style of the item if it is currently selected.
    virtual void SetCurrent(bool is_current, bool hide_entry_point_lists = true) = 0;

    // Connect signals for this file item.
    void ConnectSignals();

    // Get the filename associated with this menu item.
    const std::string& GetFilename() const;

    // Show the context menu in a default position.
    void OpenContextMenu();

    // Toggle the visibility of the renaming controls.
    void ShowRenameControls(bool is_renaming);

    // Mark the item as saved or unsaved (denoted by "*" after the filename).
    void SetIsSaved(bool is_saved);

signals:
    // Signal emitted when the file has been renamed.
    void FileRenamed(const std::string& old_file_path, const std::string& new_file_path);

    // Signal emitted when the menu item is clicked.
    void MenuItemSelected(RgMenuFileItem* selected_item);

public slots:
    // Handler invoked when a rename operation is finished.
    void HandleEnterPressed();

    // Handler invoked when the focused widget changes.
    void HandleFocusChanged(QWidget* old, QWidget* now);

private slots:
    // Handler invoked when the user clicks the item's "Open in file browser" context menu item.
    void HandleOpenInFileBrowserClicked();

    // Handler invoked when the user clicks the item's "Rename" context menu item.
    void HandleRenameClicked();

    // Handler invoked when the user wants to open the context menu for the item.
    void HandleOpenContextMenu(const QPoint& local_click_position);

protected:
    // Update the item's file label text.
    virtual void UpdateFilenameLabelText() = 0;

    // Get the rename item line edit widget.
    virtual QLineEdit* GetRenameLineEdit() = 0;

    // Get the item text label widget.
    virtual QLabel* GetItemLabel() = 0;

    // Initialize the context menu and all options within it.
    void InitializeContextMenu();

    // Renames the file, but it will remain in the same location.
    bool RenameFile();

    // Update the file path for this item.
    void UpdateFilepath(const std::string& new_file_path);

    // The full path to the file.
    std::string full_file_path_;

    // The filename of the file.
    std::string filename_;

    // The context menu to display when the user right-clicks on a file item.
    QMenu* context_menu_ = nullptr;

    // Actions for context menu.
    struct
    {
        QAction* open_containing_folder = nullptr;
        QAction* rename_file            = nullptr;
        QAction* remove_file            = nullptr;
    } context_menu_actions_;

    // Whether or not the file has been saved.
    bool is_saved_;

    // The flag to keep track of escape key being pressed.
    bool is_escape_pressed_ = false;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_FILE_ITEM_H_
