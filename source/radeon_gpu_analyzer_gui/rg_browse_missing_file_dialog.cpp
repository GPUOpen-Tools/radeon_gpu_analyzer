//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for RGA's browse missing file dialog.
//=============================================================================

// C++.
#include <cassert>

// Qt.
#include <QWidget>
#include <QDialog>
#include <QSignalMapper>
#include <QPainter>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_browse_missing_file_dialog.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"

RgBrowseMissingFileDialog::RgBrowseMissingFileDialog(RgProjectAPI api, QWidget* parent)
    : QDialog(parent)
    , project_api_(api)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Workaround to avoid browsing.
    ui_.browsePushButton->setVisible(false);
    ui_.cancelPushButton->setVisible(false);

    // Connect the signals.
    ConnectSignals();

    // Create item delegate for the list widget.
    item_delegate_ = new RgMissingFileItemDelegate();
    bool is_delegate_valid = (item_delegate_ != nullptr);
    assert(is_delegate_valid);

    if (is_delegate_valid)
    {
        // Set custom delegate for the list widget.
        ui_.fileListWidget->setItemDelegate(item_delegate_);
    }

    // Disable selection of items.
    ui_.fileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Disable the browse button until the user selects a row to update.
    ToggleBrowseButtonEnabled(false);

    // Disable the help button in the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

RgBrowseMissingFileDialog::~RgBrowseMissingFileDialog()
{
    delete item_delegate_;
}

void RgBrowseMissingFileDialog::ConnectSignals()
{
    // File list's selected row changed handler.
    QItemSelectionModel* file_list_selection_model_ = ui_.fileListWidget->selectionModel();
    bool is_connected = connect(file_list_selection_model_, &QItemSelectionModel::selectionChanged, this, &RgBrowseMissingFileDialog::HandleSelectedFilePathChanged);
    assert(is_connected);

    // Browse button.
    is_connected = connect(ui_.browsePushButton, &QPushButton::clicked, this, &RgBrowseMissingFileDialog::HandleBrowseButtonClicked);
    assert(is_connected);

    // OK button.
    is_connected = connect(ui_.okPushButton, &QPushButton::clicked, [=] { done(MissingFileDialogResult::kOK); });
    assert(is_connected);

    // Cancel button.
    is_connected = connect(ui_.cancelPushButton, &QPushButton::clicked, [=] { done(MissingFileDialogResult::kCancel); });
    assert(is_connected);

    is_connected = connect(this, &QDialog::rejected, this, &RgBrowseMissingFileDialog::HandleRejected);
    assert(is_connected);
}

void RgBrowseMissingFileDialog::ToggleBrowseButtonEnabled(bool is_enabled)
{
    ui_.browsePushButton->setEnabled(is_enabled);
}

void RgBrowseMissingFileDialog::AddFile(const std::string filename)
{
    ui_.fileListWidget->addItem(filename.c_str());
}

const std::map<std::string, std::string>& RgBrowseMissingFileDialog::GetUpdateFilePathsMap() const
{
    return updated_path_map_;
}

void RgBrowseMissingFileDialog::HandleBrowseButtonClicked()
{
    // Use the selection model to determine which path the user wants to update.
    QItemSelectionModel* selection_model = ui_.fileListWidget->selectionModel();
    assert(selection_model != nullptr);
    if (selection_model != nullptr)
    {
        QModelIndex selection_row_index = selection_model->currentIndex();
        int row_index = selection_row_index.row();

        BrowseFilePathForRow(row_index);
    }
}

void RgBrowseMissingFileDialog::BrowseFilePathForRow(int row_index)
{
    QModelIndex selection_row_index = ui_.fileListWidget->model()->index(row_index, 0);
    bool is_index_valid = selection_row_index.isValid();
    assert(is_index_valid);

    if (is_index_valid)
    {
        QVariant raw_data = selection_row_index.data(Qt::DisplayRole);

        // Extract the existing path to the missing file.
        const std::string original_file_path = raw_data.toString().toStdString();
        std::string updated_file_path  = original_file_path;

        // Show a file browser dialog so the user can update the path to the missing file.
        bool file_path_updated = RgUtils::OpenFileDialog(this, project_api_, updated_file_path);

        // Did the user provide a valid path to replace the missing file's path?
        if (file_path_updated)
        {
            bool updated_file_exists = RgUtils::IsFileExists(updated_file_path);
            assert(updated_file_exists);

            if (updated_file_exists)
            {
                // Update the map with the new file paths.
                updated_path_map_[original_file_path] = updated_file_path;

                // Update the list row to display the new updated file path.
                ui_.fileListWidget->model()->setData(selection_row_index, updated_file_path.c_str());
            }
        }
    }
}

void RgBrowseMissingFileDialog::HandleSelectedFilePathChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    // Toggle the enabledness of the Browse button if necessary.
    bool enable_browse = false;
    if (!selected.empty())
    {
        QModelIndexList selected_indices = selected.indexes();
        if (!selected_indices.empty())
        {
            QModelIndex selected_row_index = selected_indices[0];
            if (selected_row_index.isValid())
            {
                // The browse button should be clickable if the user has selected a valid row.
                enable_browse = true;
            }
        }
    }

    ToggleBrowseButtonEnabled(enable_browse);
}

void RgBrowseMissingFileDialog::HandleSelectedRowDoubleClicked(QListWidgetItem* item)
{
    int row_index = ui_.fileListWidget->row(item);

    BrowseFilePathForRow(row_index);
}

void RgBrowseMissingFileDialog::HandleRejected()
{
    setResult(MissingFileDialogResult::kCancel);
}

void RgMissingFileItemDelegate::drawDisplay(QPainter* painter, const QStyleOptionViewItem& option, const QRect &rect, const QString& text) const
{
    Q_UNUSED(option);

    bool is_painter_valid = (painter != nullptr);
    assert(is_painter_valid);

    if (is_painter_valid)
    {
        // Truncate string so it fits within rect.
        QString truncated_string = RgUtils::TruncateString(text.toStdString(), kTextTruncateLengthFront,
            kTextTruncateLengthBack, rect.width(), painter->font(), RgUtils::kExpandBack).c_str();

        // Draw text within rect.
        painter->drawText(rect, Qt::AlignVCenter, truncated_string);
    }
}

QSize RgMissingFileItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Use standard size hint implementation, but with a fixed width of 0 (width will be determined by view width).
    QSize adjusted_hint = QItemDelegate::sizeHint(option, index);
    adjusted_hint.setWidth(0);

    return adjusted_hint;
}
