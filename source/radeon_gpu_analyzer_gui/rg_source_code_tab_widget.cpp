//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for source code tab widget.
//=============================================================================

// C++.
#include <algorithm>

// Qt.
#include <QFileInfo>
#include <QPlainTextEdit>
#include <QTabBar>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_tab_widget.h"

RgSourceCodeTabWidget::RgSourceCodeTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    // Enable close buttons on tabs.
    setTabsClosable(true);

    // Connect signals.
    connect(this, &QTabWidget::currentChanged, this, &RgSourceCodeTabWidget::HandleTabChanged);
    connect(this, &QTabWidget::tabCloseRequested, this, &RgSourceCodeTabWidget::HandleTabCloseRequested);
}

void RgSourceCodeTabWidget::AddSourceFile(const std::string& filename, RgSourceCodeEditor* editor)
{
    if (editor != nullptr)
    {
        auto editor_itr = source_code_editors_.find(filename);
        if (editor_itr != source_code_editors_.end())
        {
            setCurrentWidget(editor_itr->second);
        }
        else
        {
            editor->setParent(this);
            QFileInfo info(QString::fromStdString(filename));
            int       index = addTab(editor, info.fileName());
            setTabToolTip(index, QString::fromStdString(filename));

            source_code_editors_[filename] = editor;
            setCurrentIndex(index);
        }
    }
}

void RgSourceCodeTabWidget::ClearAll()
{
    blockSignals(true);
    while (count() > 0)
    {
        removeTab(0);
    }
    blockSignals(false);

    source_code_editors_.clear();

    current_code_editor_ = nullptr;
    
    emit SourceCodeEditorChanged(nullptr);
}


void RgSourceCodeTabWidget::SetCurrentCodeEditor(RgSourceCodeEditor* editor)
{
    if (editor)
    {
        for (int i = 0; i < count(); ++i)
        {
            if (widget(i) == editor)
            {
                // Don't switch to a tab that has been closed (hidden).
                if (!isTabVisible(i))
                {
                    return;
                }
                setCurrentIndex(i);
                return;
            }
        }
    }
    else
    {
        current_code_editor_ = nullptr;
        emit SourceCodeEditorChanged(nullptr);
        return;
    }
}

RgSourceCodeEditor* RgSourceCodeTabWidget::GetCurrentCodeEditor()
{
    return current_code_editor_;
}

void RgSourceCodeTabWidget::HideEditor(const std::string& src_filename)
{
    RgSourceCodeEditor* editor_to_hide = nullptr;
    auto it = source_code_editors_.find(src_filename);
    if (it != source_code_editors_.end())
    {
        editor_to_hide = it->second;
    }

    const int tab_count = count();
    for (int i = 0; i < tab_count; i++)
    {
        RgSourceCodeEditor* editor = static_cast<RgSourceCodeEditor*>(widget(i));
        if (editor == editor_to_hide)
        {
            setTabVisible(i, false);
        }
    }
}

void RgSourceCodeTabWidget::ShowEditor(const std::string& src_filename)
{
    RgSourceCodeEditor* editor_to_show = nullptr;
    auto it = source_code_editors_.find(src_filename);
    if (it != source_code_editors_.end())
    {
        editor_to_show = it->second;
    }

    const int tab_count = count();
    for (int i = 0; i < tab_count; i++)
    {
        RgSourceCodeEditor* editor = static_cast<RgSourceCodeEditor*>(widget(i));
        if (editor == editor_to_show)
        {
            setTabVisible(i, true);
        }
    }
}

void RgSourceCodeTabWidget::HandleTabChanged(int index)
{
    current_code_editor_ = (index >= 0) ? static_cast<RgSourceCodeEditor*>(widget(index)) : nullptr;

    emit SourceCodeEditorChanged(current_code_editor_);
}

void RgSourceCodeTabWidget::HandleTabCloseRequested(int index)
{
    QWidget* tab_widget = widget(index);
    if (tab_widget != nullptr)
    {
        // Find the filepath for this editor.
        std::string closed_filepath;
        for (const auto& [filepath, editor] : source_code_editors_)
        {
            if (editor == tab_widget)
            {
                closed_filepath = filepath;
                break;
            }
        }

        // Hide the tab rather than removing it, so the editor can be
        // re-shown later (e.g. when switching binaries or via correlation).
        setTabVisible(index, false);

        if (!closed_filepath.empty())
        {
            emit SourceTabClosed(closed_filepath);
        }
    }
}

void RgSourceCodeTabWidget::HideAllEditors()
{
    const int tab_count = count();
    for (int i = 0; i < tab_count; i++)
    {
        setTabVisible(i, false);
    }
}
