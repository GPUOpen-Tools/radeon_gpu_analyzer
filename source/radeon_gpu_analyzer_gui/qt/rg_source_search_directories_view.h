//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Source Search Directories dialog.
///
/// This dialog is shown when source files referenced by a code object binary
/// cannot be found on disk after binary analysis. It extends the Include
/// Directories dialog with an informational label listing the missing files.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_SEARCH_DIRECTORIES_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_SEARCH_DIRECTORIES_VIEW_H_

// Qt.
#include <QStringList>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_include_directories_view.h"

// Forward declarations.
class QTextEdit;

class RgSourceSearchDirectoriesView : public RgIncludeDirectoriesView
{
    Q_OBJECT

public:
    RgSourceSearchDirectoriesView(const char* delimiter, const QStringList& missing_files, QWidget* parent = nullptr);
    virtual ~RgSourceSearchDirectoriesView() = default;

private:
    // Read-only text area listing the missing source files (scrollable).
    QTextEdit* info_text_edit_ = nullptr;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_SEARCH_DIRECTORIES_VIEW_H_
