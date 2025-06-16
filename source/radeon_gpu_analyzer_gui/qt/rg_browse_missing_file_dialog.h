//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RGA's browse missing file dialog.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BROWSE_MISSING_FILE_DIALOG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BROWSE_MISSING_FILE_DIALOG_H_

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QDialog>
#include <QtWidgets/QItemDelegate>

// Local.
#include "ui_rg_browse_missing_file_dialog.h"

// Forward declarations.
struct RgProject;
enum class RgProjectAPI : char;

// Delegate for the RgBrowseMissingFileDialog which draws the list of missing files with truncated text.
class RgMissingFileItemDelegate : public QItemDelegate
{
public:
    // Default constructor.
    RgMissingFileItemDelegate(QWidget* parent = nullptr) : QItemDelegate(parent) {};

    // Drawing operation for text items in the unsaved file dialog.
    virtual void drawDisplay(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text) const override;

    // Size hint for text items in the unsaved file dialog.
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Empty implementation here because we don't want to draw the focus indication.
    virtual void drawFocus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect) const override
    {
        Q_UNUSED(painter);
        Q_UNUSED(option);
        Q_UNUSED(rect);
    }
};

class RgBrowseMissingFileDialog : public QDialog
{
    Q_OBJECT

public:
    // Enum for dialog result (indicates which button was pressed).
    enum MissingFileDialogResult
    {
        kOK,
        kCancel
    };

    // Constructor and destructor.
    RgBrowseMissingFileDialog(RgProjectAPI api, QWidget* parent = nullptr);
    ~RgBrowseMissingFileDialog();

    // Add a file to the unsaved file list.
    void AddFile(const std::string filename);

    // Retrieve the map which associates original filepaths with the new ones chosen by the user.
    const std::map<std::string, std::string>& GetUpdateFilePathsMap() const;

private slots:
    // Handler invoked when the user clicks the Browse button in the dialog.
    void HandleBrowseButtonClicked();

    // Handler invoked when the user changes the selected row in the list of missing files.
    void HandleSelectedFilePathChanged(const QItemSelection &selected, const QItemSelection &deselected);

    // Handler invoked when the user double-clicks on a row in the list of missing files.
    void HandleSelectedRowDoubleClicked(QListWidgetItem* item);

    // Handler invoked when the user clicks on the "X" in the title bar to close the dialog box.
    void HandleRejected();

private:
    // Connect the signals.
    void ConnectSignals();

    // Display a file browser to the user, where they can choose to replace the path at the given row.
    void BrowseFilePathForRow(int row_index);

    // Toggle the enabledness of the Browse button.
    void ToggleBrowseButtonEnabled(bool is_enabled);

    // A map that associated the original file path with a new one that the user chose to replace it.
    std::map<std::string, std::string> updated_path_map_;

    // Delegate for drawing list items in the file list widget.
    RgMissingFileItemDelegate* item_delegate_ = nullptr;

    // The project that references missing files.
    RgProjectAPI project_api_;

    // The Qt interface for the dialog.
    Ui::RgBrowseMissingFileDialog ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BROWSE_MISSING_FILE_DIALOG_H_
