//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for source code tab widget.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_CODE_TAB_WIDGET_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_CODE_TAB_WIDGET_H_

// C++.
#include <map>
#include <set>
#include <vector>

// Qt.
#include <QTabWidget>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_source_code_editor.h"

class RgSourceCodeTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    // Ctor
    explicit RgSourceCodeTabWidget(QWidget* parent = nullptr);

    // Add a source code file in a new tab.
    void AddSourceFile(const std::string& filename, RgSourceCodeEditor* editor);

    // Remove all source code editors.
    void ClearAll();

    // Set current widget externally.
    void SetCurrentCodeEditor(RgSourceCodeEditor* editor);

    // Get current code editor.
    RgSourceCodeEditor* GetCurrentCodeEditor();

    // Calls a function on each of the editors.
    template <typename Function>
    void ForEachEditor(Function&& f)
    {
        for (const auto& [_, editor] : source_code_editors_)
        {
            if (editor != nullptr)
            {
                f(editor);
            }
        }
    }
    // Hide all source code editor tabs.
    void HideAllEditors();

    // Hide a source code editor tab by filename.
    void HideEditor(const std::string& src_filename);

    // Show source code editor tabs relevant to.
    void ShowEditor(const std::string& src_filename);

signals:
    // Signal emitted on changing code editor tab.
    void SourceCodeEditorChanged(RgSourceCodeEditor* widget);

    // Signal emitted when a source tab is closed by the user.
    void SourceTabClosed(const std::string& filepath);

private slots:
    // Current widget is changed.
    void HandleTabChanged(int index);

    // A tab's close button was clicked.
    void HandleTabCloseRequested(int index);

private:
    // Source code filename to source code editor object map.
    std::map<std::string, RgSourceCodeEditor*> source_code_editors_;

    // The instance of the RgSourceCodeEditor being used currently.
    RgSourceCodeEditor* current_code_editor_ = nullptr;

};

#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_CODE_TAB_WIDGET_H_
