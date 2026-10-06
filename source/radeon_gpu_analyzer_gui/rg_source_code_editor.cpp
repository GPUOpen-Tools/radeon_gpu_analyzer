//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for source code editor.
//=============================================================================
// C++.
#include <cassert>

// Qt.
#include <QtWidgets>
#include <QMenu>

// QtCommon.
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_editor.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgSourceCodeEditor::RgSourceCodeEditor(QWidget* parent, ShaderSourceLanguage lang)
    : ShaderSourceCodeViewer(parent, lang)
{
    // The parent class is read only so make sure the text can be edited.
    setReadOnly(false);

    // Set the default font.
    QTextDocument* doc = this->document();
    if (doc != nullptr)
    {
        QFont font = doc->defaultFont();
        font.setFamily(kStrBuildViewFontFamily);
        font.setPointSize(kBuildViewFontSize);
        doc->setDefaultFont(font);
    }

    // Set up the open header file action.
    open_header_file_action_ = new QAction(tr(kStrSourceEditorContextMenuOpenHeader), this);

    // Cut action.
    cut_text_action_ = new QAction(tr(kStrSourceEditorContextMenuCut), this);
    cut_text_action_->setShortcut(QKeySequence(kSourceEditorHotkeyContextMenuCut));

    // Paste action.
    paste_text_action_ = new QAction(tr(kStrSourceEditorContextMenuPaste), this);
    paste_text_action_->setShortcut(QKeySequence(kSourceEditorHotkeyContextMenuPaste));

    // Open header file.
    connect(open_header_file_action_, &QAction::triggered, this, &RgSourceCodeEditor::HandleOpenHeaderFile);

    connect(cut_text_action_, &QAction::triggered, this, &QPlainTextEdit::cut);

    connect(paste_text_action_, &QAction::triggered, this, &QPlainTextEdit::paste);

    // The context menu as well as the copy and select all options should have been initialized by the base class.
    assert(copy_text_action_ != nullptr);
    assert(select_all_text_action_ != nullptr);
    assert(context_menu_ != nullptr);
    if (context_menu_ != nullptr && select_all_text_action_ != nullptr && copy_text_action_ != nullptr)
    {
        // Clear and add back all the actions so that the new actions can be placed in the menu in the desired order.
        context_menu_->clear();
        context_menu_->addAction(open_header_file_action_);
        context_menu_->addSeparator();
        context_menu_->addAction(cut_text_action_);
        context_menu_->addAction(copy_text_action_);
        context_menu_->addAction(paste_text_action_);
        context_menu_->addSeparator();
        context_menu_->addAction(select_all_text_action_);
        setContextMenuPolicy(Qt::CustomContextMenu);
    }
}

void RgSourceCodeEditor::HandleOpenHeaderFile()
{
    QString line_text;
    bool    is_valid = GetCurrentLineText(line_text);
    if (is_valid)
    {
        // Parse the line.
        if (IsIncludeDirectiveLine(line_text))
        {
            bool is_double_quoted = (line_text.count("\"") == 2);
            bool is_triangular =
                !is_double_quoted && (line_text.indexOf("<") != -1) && (line_text.indexOf(">") != -1) && (line_text.indexOf("<") < line_text.indexOf(">"));
            if (is_double_quoted)
            {
                // Extract the file name.
                QStringList line_broken = line_text.split("\"");
                if (line_broken.size() >= 2)
                {
                    QString filename = line_broken[1];

                    // Fire the signal: user requested to open header file.
                    emit OpenHeaderFileRequested(filename);
                }
            }
            else if (is_triangular)
            {
                // Extract the part that is after the first < character.
                QStringList line_broken_a = line_text.split("<");
                if (line_broken_a.size() >= 2)
                {
                    // Extract the part that is before the > character.
                    QStringList line_broken_b = line_broken_a.at(1).split(">");
                    if (line_broken_b.size() >= 1)
                    {
                        // Fire the signal: user requested to open header file.
                        QString filename = line_broken_b[0];
                        emit    OpenHeaderFileRequested(filename);
                    }
                }
            }
        }
    }
}

void RgSourceCodeEditor::ShowContextMenu(const QPoint& pt)
{
    // Is the open header file action relevant.
    QString line_text;

    // Set the open header action to enabled
    // only when the line is an include directive.
    open_header_file_action_->setEnabled(IsIncludeDirectiveLine(line_text));

    ShaderSourceCodeViewer::ShowContextMenu(pt);
}

void RgSourceCodeEditor::HighlightCursorLine(QList<QTextEdit::ExtraSelection>& selections)
{
    if (!isReadOnly())
    {
        ShaderSourceCodeViewer::HighlightCursorLine(selections);
    }
}

void RgSourceCodeEditor::HighlightCorrelatedSourceLines(QList<QTextEdit::ExtraSelection>& selections)
{
    if (!isReadOnly())
    {
        ShaderSourceCodeViewer::HighlightCorrelatedSourceLines(selections);
    }
}

void RgSourceCodeEditor::mousePressEvent(QMouseEvent* event)
{
    // Disable disassembly view's scroll bar signals.
    // This is needed because when the user clicks on source code editor,
    // the disassembly view's scroll bars emit a signal, causing the
    // disassembly view's border to be colored red, and now the user
    // will see both the source code editor and the disassembly view
    // with a red border around it.
    emit DisableScrollbarSignals();

    emit SourceCodeEditorFocusInEvent();

    // In read-only placeholder editors, detect clicks on the "Click here" link line.
    // cursorForPosition maps clicks in empty space below text to the last block,
    // so we also check that the click is within the block's visual bounding rect.
    if (isReadOnly() && event->button() == Qt::LeftButton)
    {
        QTextCursor cursor = cursorForPosition(event->pos());
        QTextBlock  block  = cursor.block();
        if (block.text().contains(kStrMissingSourceClickHereText))
        {
            QRectF block_rect = blockBoundingGeometry(block).translated(contentOffset());
            if (block_rect.contains(QPointF(event->pos())))
            {
                emit OpenSourceSearchDirectoriesRequested();
                emit EnableScrollbarSignals();
                return;
            }
        }
    }

    ShaderSourceCodeViewer::mousePressEvent(event);

    // Enable disassembly view's scroll bar signals.
    emit EnableScrollbarSignals();
}

void RgSourceCodeEditor::mouseMoveEvent(QMouseEvent* event)
{
    // In read-only placeholder editors, show a hand cursor over the "Click here" link line.
    if (isReadOnly())
    {
        QTextCursor cursor = cursorForPosition(event->pos());
        QTextBlock  block  = cursor.block();
        QRectF      block_rect = blockBoundingGeometry(block).translated(contentOffset());
        if (block.text().contains(kStrMissingSourceClickHereText) && block_rect.contains(QPointF(event->pos())))
        {
            viewport()->setCursor(Qt::PointingHandCursor);
        }
        else
        {
            viewport()->setCursor(Qt::ArrowCursor);
        }
    }

    ShaderSourceCodeViewer::mouseMoveEvent(event);
}
