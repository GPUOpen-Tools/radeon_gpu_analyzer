// C++.
#include <cassert>

// Qt.
#include <QScrollBar>
#include <QTextEdit>
#include <QTextBlock>
#include <QMouseEvent>

// Local.
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/qt/rg_cli_output_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu.h"

// Local constants
static const QString  kStrCmplrErrorToken          = ": error:";
static const QString  kStrCmplrWarningToken        = ": warning:";
static const QString  kStrCmplrNoteToken           = ": note:";
static const QChar    kCharCmlprErrorDelimiter     = ':';
static const QChar    kCharLParenthesis             = '(';
static const QChar    kCharRParenthesis             = ')';

RgCliOutputView::RgCliOutputView(QWidget *parent) :
    QWidget(parent)
{
    ui_.setupUi(this);

    // Use the output text edit as the focus proxy for this view.
    setFocusProxy(ui_.outputTextEdit);

    // To receive focus, the user must tab or click onto the output window widget.
    setFocusPolicy(Qt::StrongFocus);

    // Use monospace font style so that characters align.
    QFont font("unexistent");
    font.setStyleHint(QFont::Monospace);
    ui_.outputTextEdit->setFont(font);

    // Set the status and tool tips.
    ui_.clearOutputPushButton->setStatusTip(kStrOutputWindowClearButtonStatustip);
    ui_.clearOutputPushButton->setToolTip(kStrOutputWindowClearButtonTooltip);

    // Connect the signals.
    ConnectSignals();

    // Set the event interceptor for TextEdit visible area.
    ui_.outputTextEdit->viewport()->installEventFilter(this);

    // Set the mouse cursor to pointing hand cursor.
    SetCursor();

    // Create shortcut actions.
    CreateActions();
}

void RgCliOutputView::CreateActions()
{
    // Tab key navigation.
    tab_key_action_ = new QAction(this);
    tab_key_action_->setShortcut(QKeySequence(kActionHotkeyFileMenuActivateTab));
    tab_key_action_->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    addAction(tab_key_action_);
    bool is_connected = connect(tab_key_action_, &QAction::triggered, this, &RgCliOutputView::HandleTabFocusPressed);
    assert(is_connected);

    // Shift+Tab key navigation.
    shift_tab_key_action_ = new QAction(this);
    shift_tab_key_action_->setShortcut(QKeySequence(kActionHotkeyFileMenuActivateShiftTab));
    shift_tab_key_action_->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    addAction(shift_tab_key_action_);
    is_connected = connect(shift_tab_key_action_, &QAction::triggered, this, &RgCliOutputView::HandleShiftTabFocusPressed);
    assert(is_connected);
}

void RgCliOutputView::HandleTabFocusPressed()
{
    // Switch focus to the next child widget,
    // unless the focus is on the last child widget,
    // in which case switch focus to file menu.
    if (current_sub_widget_ == CliOutputWindowSubWidgets::kOutputWindow)
    {
        ui_.clearOutputPushButton->setFocus();
        current_sub_widget_ = CliOutputWindowSubWidgets::kClearWindowButton;
    }
    else if (current_sub_widget_ == CliOutputWindowSubWidgets::kClearWindowButton)
    {
        current_sub_widget_ = CliOutputWindowSubWidgets::kOutputWindow;
        emit FocusNextView();
    }
}

void RgCliOutputView::HandleShiftTabFocusPressed()
{
    // Switch focus to the next child widget,
    // unless the focus is on the last child widget,
    // in which case switch focus to file menu.
    if (current_sub_widget_ == CliOutputWindowSubWidgets::kOutputWindow)
    {
        emit FocusRawTextDisassemblyPushButton();
    }
    else if (current_sub_widget_ == CliOutputWindowSubWidgets::kClearWindowButton)
    {
        current_sub_widget_ = CliOutputWindowSubWidgets::kOutputWindow;
        emit FocusOutputWindow();
    }
}

void RgCliOutputView::mousePressEvent(QMouseEvent *event)
{
    current_sub_widget_ = CliOutputWindowSubWidgets::kOutputWindow;

    // Pass the event onto the base class.
    QWidget::mousePressEvent(event);
}

std::string RgCliOutputView::GetText() const
{
    return ui_.outputTextEdit->toPlainText().toStdString();
}

void RgCliOutputView::focusInEvent(QFocusEvent*)
{
    current_sub_widget_ = CliOutputWindowSubWidgets::kOutputWindow;
}

void RgCliOutputView::ClearText()
{
    ui_.outputTextEdit->clear();
}

bool RgCliOutputView::eventFilter(QObject* object, QEvent* event)
{
    assert(event != nullptr);
    if (event != nullptr)
    {
        // Intercept "mouse double-click" events for TextEdit area.
        if (event->type() == QMouseEvent::MouseButtonDblClick)
        {
            const QPoint  clicked_pos = static_cast<QMouseEvent*>(event)->pos();
            QTextCursor   text_pos = ui_.outputTextEdit->cursorForPosition(clicked_pos);
            SwitchToErrorLocation(text_pos.blockNumber());
        }
        else if (event->type() == QMouseEvent::MouseButtonPress)
        {
            current_sub_widget_ = CliOutputWindowSubWidgets::kOutputWindow;
        }
    }

    // Continue default processing for all other events.
    return QObject::eventFilter(object, event);
}

void RgCliOutputView::ConnectSignals()
{
    bool is_connected = connect(this, &RgCliOutputView::EmitSetText, this, &RgCliOutputView::HandleAppendText);
    assert(is_connected);

    is_connected = connect(ui_.clearOutputPushButton, &QPushButton::clicked, this, &RgCliOutputView::HandleClearClicked);
    assert(is_connected);
}

void RgCliOutputView::ScrollToBottom()
{
    QScrollBar* scroll_bar = ui_.outputTextEdit->verticalScrollBar();
    scroll_bar->setValue(scroll_bar->maximum());
}

void RgCliOutputView::HandleAppendText(const QString& str)
{
    ui_.outputTextEdit->append(str);
    ScrollToBottom();

    // Focus on the output window when text is appended (this is the ONLY time the window will receive focus).
    setFocus();
}

void RgCliOutputView::HandleClearClicked()
{
    ui_.outputTextEdit->clear();
}

void RgCliOutputView::HandleBuildStarted()
{
    // Do not allow to clear the output window during a build.
    ui_.clearOutputPushButton->setEnabled(false);
}

void RgCliOutputView::HandleBuildEnded()
{
    // Re-enable the clear button after the build is over.
    ui_.clearOutputPushButton->setEnabled(true);
}

void RgCliOutputView::SetCursor()
{
    // Set the mouse cursor to pointing hand cursor.
    ui_.clearOutputPushButton->setCursor(Qt::PointingHandCursor);
    ui_.viewMaximizeButton->setCursor(Qt::PointingHandCursor);
}

//
// Parse the error location generated by CLI Compiler. Returns a pair {filename, line}.
//
static bool ParseErrorLocation(const QStringView& loc, std::pair<QStringView&, int&> parsed_loc)
{
    // The clang and lld may produce 2 formats of error location messages:
    // 1.   C:\OpenCL\test.cl:4:7: error: ...
    //      `-- filename---' ^ ^
    //                  line--' `--column
    //
    // 2.   ...(C:\OpenCL\test.cl:4)...
    //          `-- filename---' ^
    //                      line--'
    //
    // Note that the source file path may contain symbols lile ':', '(', ')', etc.

    bool  found = false;

    // I. Try matching the 1st pattern.
    int  column_offset, parent_offset = loc.lastIndexOf(kCharRParenthesis);
    if (parent_offset != -1 && (column_offset = loc.lastIndexOf(kCharCmlprErrorDelimiter, parent_offset)) != -1)
    {
        // Parse the "loc" string backwards trying to find '(' matching the ')' found at rParenthOffset.
        int  parent_stack_count = 1, offset = parent_offset;
        while (offset > 0 && parent_stack_count > 0)
        {
            const QChar sym = loc.at(--offset);
            parent_stack_count += (sym == kCharLParenthesis ? -1 : (sym == kCharRParenthesis ? 1 : 0));
        }

        if (parent_stack_count == 0 && column_offset > offset)
        {
            QStringView line_num_str = loc.mid(column_offset + 1, parent_offset - column_offset - 1);
            parsed_loc.second        = line_num_str.toInt(&found);
            if (found)
            {
                std::get<0>(parsed_loc) = loc.mid(offset + 1, column_offset - offset - 1);
            }
        }
    }

    // II. Try mathching the 2nd pattern.
    if (!found)
    {
        int offset, line_start_offset = 0, line_end_offset;

        if ((offset = loc.indexOf(kStrCmplrErrorToken)) != -1 ||
            (offset = loc.indexOf(kStrCmplrWarningToken)) != -1 ||
            (offset = loc.indexOf(kStrCmplrNoteToken)) != -1)
        {
            // 1. Skip the column number and parse the line number.
            if ((line_end_offset = loc.lastIndexOf(kCharCmlprErrorDelimiter, offset - 1)) != -1 &&
                (line_start_offset = loc.lastIndexOf(kCharCmlprErrorDelimiter, line_end_offset - 1)) != -1)
            {
                QStringView line_str    = loc.mid(line_start_offset + 1, line_end_offset - line_start_offset - 1);
                std::get<1>(parsed_loc) = line_str.toInt(&found);
            }

            // 2. Copy the file name.
            if (found)
            {
                std::get<0>(parsed_loc) = loc.left(line_start_offset);
            }
        }
    }

    return found;
}

void RgCliOutputView::SwitchToErrorLocation(int block_num) const
{
    if (block_num > 0)
    {
        if (QTextDocument* doc = ui_.outputTextEdit->document())
        {
            QTextBlock    block = doc->findBlockByNumber(block_num);
            QString       block_text = block.text();
            QStringView   file_path;
            int           line = 0;

            if (!block_text.isEmpty() && ParseErrorLocation(block_text, { file_path, line }))
            {
                emit SwitchToFile(file_path.toString().toStdString(), line);
            }
        }
    }
}
