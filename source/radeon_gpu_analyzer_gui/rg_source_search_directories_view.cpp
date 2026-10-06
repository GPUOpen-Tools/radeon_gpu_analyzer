//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Source Search Directories dialog.
//=============================================================================

// Qt.
#include <QFileInfo>
#include <QTextEdit>
#include <QVBoxLayout>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_source_search_directories_view.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

static const int kInfoTextEditMaxHeight = 160;
static const int kDialogMinHeight       = 400;

RgSourceSearchDirectoriesView::RgSourceSearchDirectoriesView(const char* delimiter, const QStringList& missing_files, QWidget* parent)
    : RgIncludeDirectoriesView(delimiter, parent)
{
    // Set the custom window title.
    setWindowTitle(kStrSourceSearchDirsDialogTitle);

    // Build the informational text listing all missing source files.
    QString info_text = QString(kStrSourceSearchDirsDialogInfoPrefix) + "\n\n";
    for (const QString& file_path : missing_files)
    {
        info_text += "  - " + file_path + "\n";
    }
    info_text += "\n" + QString(kStrSourceSearchDirsDialogInfoSuffix);

    // Create a read-only text area so the file list scrolls when it's long.
    info_text_edit_ = new QTextEdit(this);
    info_text_edit_->setPlainText(info_text);
    info_text_edit_->setReadOnly(true);
    info_text_edit_->setFrameShape(QFrame::NoFrame);
    info_text_edit_->setMaximumHeight(kInfoTextEditMaxHeight);
    info_text_edit_->setContentsMargins(10, 5, 10, 10);

    // Insert the text area at the top of the dialog's main vertical layout.
    // The layout is "verticalLayout_4" defined in rg_ordered_list_view_dialog.ui.
    ui_.verticalLayout_4->insertWidget(0, info_text_edit_);

    // Increase the dialog's minimum height to accommodate the info area.
    setMinimumHeight(kDialogMinHeight);
}
