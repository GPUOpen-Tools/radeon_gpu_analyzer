//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the raw text disassembly view.
//=============================================================================

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_raw_text_disassembly_view.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"

RgRawTextDisassemblyView::RgRawTextDisassemblyView(QWidget* parent)
    : QPlainTextEdit(parent)
{
    // Set the border color.
    setObjectName("rawTextDisassemblyViewEditor");

    // Set the default font.
    QTextDocument* doc = this->document();
    if (doc != nullptr)
    {
        QFont font = doc->defaultFont();
        font.setFamily(kStrBuildViewFontFamily);
        font.setPointSize(kBuildViewFontSize);
        doc->setDefaultFont(font);

        // A tab is the same width as 4 spaces.
        QFontMetrics metrics(font);
        int          tab_width = metrics.horizontalAdvance(' ') * 4;
        setTabStopDistance(tab_width);
    }

    // Configure the word wrap mode.
    setWordWrapMode(QTextOption::NoWrap);

    // Disable accepting dropped files.
    setAcceptDrops(false);

    // Copy action.
    copy_text_action_ = new QAction(tr(kStrSourceEditorContextMenuCopy), this);
    copy_text_action_->setShortcut(QKeySequence(kSourceEditorHotkeyContextMenuCopy));

    // Select All action.
    select_all_text_action_ = new QAction(tr(kStrSourceEditorContextMenuSelectAll), this);

    // Copy action.
    bool is_connected = connect(copy_text_action_, &QAction::triggered, this, &QPlainTextEdit::copy);
    assert(is_connected);

    // Select All action.
    is_connected = connect(select_all_text_action_, &QAction::triggered, this, &QPlainTextEdit::selectAll);
    assert(is_connected);

    context_menu_ = this->createStandardContextMenu();
    assert(context_menu_ != nullptr);
    if (context_menu_ != nullptr)
    {
        // Reconstruct the context menu.
        context_menu_->clear();
        context_menu_->addAction(copy_text_action_);
        context_menu_->addSeparator();
        context_menu_->addAction(select_all_text_action_);
        this->setContextMenuPolicy(Qt::CustomContextMenu);

        // Set hand pointer for the context menu.
        context_menu_->setCursor(Qt::PointingHandCursor);

        // Connect the signal for showing the context menu.
        is_connected = connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
        assert(is_connected);
    }
}

void RgRawTextDisassemblyView::setText(const QString& txt)
{
    QTextDocument* doc = this->document();
    if (doc != nullptr)
    {
        // Set the text.
        doc->setPlainText(txt);

        // Set the cursor to the first line and column.
        this->moveCursor(QTextCursor::Start);
        this->ensureCursorVisible();
    }
}

void RgRawTextDisassemblyView::clearText(const QString& txt)
{
    Q_UNUSED(txt);

    QTextDocument* doc = this->document();
    if (doc != nullptr && !doc->isEmpty())
    {
        doc->clear();
    }
}

void RgRawTextDisassemblyView::ShowContextMenu(const QPoint& pt)
{
    Q_UNUSED(pt);


    // Only enable cut, copy if there is text selected.
    QTextCursor cursor        = this->textCursor();
    bool        has_selection = cursor.hasSelection();
    copy_text_action_->setEnabled(has_selection);

    // Show the context menu to the user where the mouse is.
    assert(context_menu_ != nullptr);
    if (context_menu_ != nullptr)
    {
        context_menu_->exec(QCursor::pos());
    }
}
