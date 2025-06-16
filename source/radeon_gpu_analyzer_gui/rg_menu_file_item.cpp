//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Item widget in RGA Build view's File Menu.
//=============================================================================

// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QApplication>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgMenuFileItem::RgMenuFileItem(const std::string& full_file_path, RgMenu* parent)
    : RgMenuItem(parent)
    , full_file_path_(full_file_path)
{
    // Initialize the context menu for right-clicks on the item.
    InitializeContextMenu();

    // Connect signals to slots.
    ConnectSignals();
}

void RgMenuFileItem::ConnectSignals()
{
    // Connect a FocusChanged handler so we know when a filename change is completed.
    bool is_connected = connect(qApp, &QApplication::focusChanged, this, &RgMenuFileItem::HandleFocusChanged);
    assert(is_connected);

    // Connect the item's "Open in file browser" menu item.
    is_connected = connect(context_menu_actions_.open_containing_folder, &QAction::triggered, this, &RgMenuFileItem::HandleOpenInFileBrowserClicked);
    assert(is_connected);

    // Connect the item's "Rename" menu item.
    is_connected = connect(context_menu_actions_.rename_file, &QAction::triggered, this, &RgMenuFileItem::HandleRenameClicked);
    assert(is_connected);

    // Connect the handler responsible for showing the item's context menu.
    is_connected = connect(this, &QWidget::customContextMenuRequested, this, &RgMenuFileItem::HandleOpenContextMenu);
    assert(is_connected);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
}

const std::string& RgMenuFileItem::GetFilename() const
{
    return full_file_path_;
}

void RgMenuFileItem::OpenContextMenu()
{
    QPoint centerPoint(width() / 2, height() / 2);

    // Open the context menu on a default centered point.
    HandleOpenContextMenu(centerPoint);
}

void RgMenuFileItem::ShowRenameControls(bool is_renaming)
{
    QLineEdit* line_edit = GetRenameLineEdit();
    QLabel* item_label = GetItemLabel();

    // Disable signals from the file name line edit and the application for now,
    // so updating line edit and setting focus to it do not cause another
    // signal to fire, causing an infinite loop.
    QSignalBlocker signalBlockerLineEdit(line_edit);
    QSignalBlocker signalBlockerApplication(qApp);

    // Swap the visibility of the filename label and line edit.
    if (is_renaming)
    {
        item_label->setVisible(false);
        line_edit->setText(filename_.c_str());
        line_edit->setVisible(true);

        std::string filename_only;
        [[maybe_unused]] bool got_filename = RgUtils::ExtractFileName(filename_, filename_only, false);
        assert(got_filename);

        // Focus on the widget with the filename selected so the user can start typing immediately.
        line_edit->setFocus();
        line_edit->setSelection(0, static_cast<int>(filename_only.length()));

        // Set cursor to IBeam cursor.
        setCursor(Qt::IBeamCursor);
    }
    else
    {
        item_label->setVisible(true);
        line_edit->setVisible(false);

        // Set cursor to Arrow cursor.
        setCursor(Qt::ArrowCursor);
    }
}

void RgMenuFileItem::SetIsSaved(bool is_saved)
{
    // Only refresh if there is a change.
    if (is_saved_ != is_saved)
    {
        is_saved_ = is_saved;
        UpdateFilenameLabelText();
    }
    else
    {
        is_saved_ = is_saved;
    }
}

void RgMenuFileItem::UpdateFilepath(const std::string& new_file_path)
{
    full_file_path_ = new_file_path;

    if (!new_file_path.empty())
    {
        // Only display the filename in the interface- not the full path to the file.
        [[maybe_unused]] bool is_ok = RgUtils::ExtractFileName(new_file_path, filename_);
        assert(is_ok);
    }
    else
    {
        // When the full path to the file is cleared, also clear the filename string.
        filename_.clear();
    }

    // Update the view to display the latest filename.
    UpdateFilenameLabelText();
}

void RgMenuFileItem::HandleEnterPressed()
{
    // Handle the file name change.
    RenameFile();
}

void RgMenuFileItem::HandleFocusChanged(QWidget* old, QWidget* now)
{
    Q_UNUSED(now);

    if (old != nullptr)
    {
        // If the control that lost focus was the renaming QLineEdit, finish the item rename.
        if (old == GetRenameLineEdit())
        {
            RenameFile();
        }
    }
}

void RgMenuFileItem::HandleOpenInFileBrowserClicked()
{
    std::string file_directory;
    bool got_directory = RgUtils::ExtractFileDirectory(GetFilename(), file_directory);
    assert(got_directory);

    if (got_directory)
    {
        // Open a system file browser window pointing to the given directory.
        RgUtils::OpenFolderInFileBrowser(file_directory);
    }
}

void RgMenuFileItem::HandleRenameClicked()
{
    // Show the file item renaming controls.
    ShowRenameControls(true);
}

void RgMenuFileItem::HandleOpenContextMenu(const QPoint& widget_click_position)
{
    // Only open the context menu for file items that aren't empty.
    if (!full_file_path_.empty())
    {
        // Convert the widget's local click position to the global screen position.
        const QPoint click_point = mapToGlobal(widget_click_position);

        // Open the context menu at the user's click position.
        context_menu_->exec(click_point);
    }
}

void RgMenuFileItem::InitializeContextMenu()
{
    // Create the context menu instance.
    context_menu_ = new QMenu(this);

    // Set the cursor for the context menu.
    context_menu_->setCursor(Qt::PointingHandCursor);

    // Create the menu items to insert into the context menu.
    context_menu_actions_.open_containing_folder = new QAction(kStrFileContextMenuOpenContainingFolder, this);
    context_menu_->addAction(context_menu_actions_.open_containing_folder);

    // Add a separator between the current menu items.
    context_menu_->addSeparator();

    // Create the rename action and add it to the menu.
    context_menu_actions_.rename_file = new QAction(kStrFileContextMenuRenameFile, this);
    context_menu_->addAction(context_menu_actions_.rename_file);

    // Create the remove action and add it to the menu.
    context_menu_actions_.remove_file = new QAction(kStrFileContextMenuRemoveFile, this);
    context_menu_->addAction(context_menu_actions_.remove_file);
}

bool RgMenuFileItem::RenameFile()
{
    bool is_file_renamed = false;

    if (!is_escape_pressed_)
    {
        QLineEdit* line_edit = GetRenameLineEdit();

        // The new filename is whatever the user left in the renaming QLineEdit.
        const std::string new_file_name = line_edit->text().toStdString();

        // Only attempt a rename if the user has altered the filename string.
        bool filename_changed = filename_.compare(new_file_name) != 0;

        // If the current filename differs from what the user left in the QLineEdit, update the filename.
        if (filename_changed)
        {
            // The renamed file will live in the same location as the old one.
            std::string file_folder_path;
            [[maybe_unused]] bool got_folder = RgUtils::ExtractFileDirectory(full_file_path_, file_folder_path);
            assert(got_folder);

            // Generate the full path to where the new file lives.
            std::string new_file_path;
            RgUtils::AppendFileNameToPath(file_folder_path, new_file_name, new_file_path);

            // Disable signals from the file name line edit and the application for now,
            // so updating line edit and setting focus to it do not cause another
            // signal to fire, causing an infinite loop.
            QSignalBlocker signal_blocker_line_edit(line_edit);
            QSignalBlocker signal_blocker_application(qApp);
            if (RgUtils::IsValidFileName(new_file_name))
            {
                if (!RgUtils::IsFileExists(new_file_path))
                {
                    // Rename the file on disk.
                    RgUtils::RenameFile(full_file_path_, new_file_path);

                    // Signal to the file menu that the file path has changed.
                    emit FileRenamed(full_file_path_, new_file_path);

                    // Update the file path so the item will display the correct filename in the menu.
                    UpdateFilepath(new_file_path);

                    // The file on disk was successfully renamed.
                    is_file_renamed = true;
                }
                else
                {
                    // Show an error message stating that the rename failed because a
                    // file with the same name already exists in the same location.
                    std::stringstream msg;
                    msg << kStrErrCannotRenameFile;
                    msg << new_file_name;
                    msg << kStrErrCannotRenameFileAlreadyExists;

                    RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
                }
            }
            else
            {
                std::stringstream msg;
                // Show an error message stating that the rename failed because
                // the given name is invalid.
                if (new_file_name.empty())
                {
                    msg << kStrErrCannotRenameFileBlankFilename;
                }
                else
                {
                    msg << kStrErrCannotRenameFile;
                    msg << new_file_name;
                    msg << kStrErrCannotRenameFileIllegalFilename;
                }

                RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
            }
        }

        // Re-enable the filename editing controls, since the user gets another chance to attempt a rename.
        if (!is_file_renamed && filename_changed)
        {
            // Toggle the item back to editing mode, showing the original filename.
            ShowRenameControls(true);
        }
        else
        {
            // Toggle the item back to being read-only, showing the updated filename.
            ShowRenameControls(false);
        }
    }
    else
    {
        is_escape_pressed_ = false;
    }
    return is_file_renamed;
}
