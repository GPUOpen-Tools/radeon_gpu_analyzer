// C++.
#include <cassert>

// Qt.
#include <QtWidgets>
#include <QMenu>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_editor.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// Use a light yellow to highlight the line that the cursor is on.
static const QColor kColorHighlightedRow = QColor(Qt::yellow).lighter(170);

RgSourceCodeEditor::RgSourceCodeEditor(QWidget* parent, RgSrcLanguage lang) : QPlainTextEdit(parent)
{
    line_number_area_ = new LineNumberArea(this);

    // The duration of time between the text cursor blinking on and off.
    static const int kCursorBlinkToggleDurationMs = 500;

    // Create the blinking cursor update timer.
    cursor_blink_timer_ = new QTimer(this);
    cursor_blink_timer_->setInterval(kCursorBlinkToggleDurationMs);
    cursor_blink_timer_->start();

    // Connect signals.
    ConnectSignals();

    // Set the border color.
    setObjectName("sourceCodeEditor");
    setStyleSheet("#sourceCodeEditor {border: 1px solid black;}");

    // Create the syntax highlighter.
    if (lang != RgSrcLanguage::Unknown)
    {
        syntax_highlighter_ = new RgSyntaxHighlighter(document(), lang);
    }

    UpdateLineNumberAreaWidth(0);

    // Initialize rendering of highlighted lines within the editor.
    UpdateCursorPosition();

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
        int tab_width = metrics.width(' ') * 4;
        setTabStopWidth(tab_width);
    }

    // Configure the word wrap mode.
    setWordWrapMode(QTextOption::NoWrap);

    // Disable accepting dropped files.
    setAcceptDrops(false);

    // Set up the open header file action.
    open_header_file_action_ = new QAction(tr(kStrSourceEditorContextMenuOpenHeader), this);

    // Cut action.
    cut_text_action_ = new QAction(tr(kStrSourceEditorContextMenuCut), this);
    cut_text_action_->setShortcut(QKeySequence(kSourceEditorHotkeyContextMenuCut));

    // Copy action.
    copy_text_action_ = new QAction(tr(kStrSourceEditorContextMenuCopy), this);
    copy_text_action_->setShortcut(QKeySequence(kSourceEditorHotkeyContextMenuCopy));

    // Paste action.
    paste_text_action_ = new QAction(tr(kStrSourceEditorContextMenuPaste), this);
    paste_text_action_->setShortcut(QKeySequence(kSourceEditorHotkeyContextMenuPaste));

    // Select All action.
    select_all_text_action_ = new QAction(tr(kStrSourceEditorContextMenuSelectAll), this);
    cut_text_action_->setShortcut(QKeySequence(kSourceEditorHotkeyContextMenuCut));

    // Open header file.
    bool is_connected = connect(open_header_file_action_, &QAction::triggered, this, &RgSourceCodeEditor::HandleOpenHeaderFile);
    assert(is_connected);

    // Cut action.
    is_connected = connect(cut_text_action_, &QAction::triggered, this, &QPlainTextEdit::cut);
    assert(is_connected);

    // Copy action.
    is_connected = connect(copy_text_action_, &QAction::triggered, this, &QPlainTextEdit::copy);
    assert(is_connected);

    // Paste action.
    is_connected = connect(paste_text_action_, &QAction::triggered, this, &QPlainTextEdit::paste);
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
        context_menu_->addAction(open_header_file_action_);
        context_menu_->addSeparator();
        context_menu_->addAction(cut_text_action_);
        context_menu_->addAction(copy_text_action_);
        context_menu_->addAction(paste_text_action_);
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

int RgSourceCodeEditor::LineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    int space = 15 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void RgSourceCodeEditor::ScrollToLine(int line_number)
{
    // Compute the last visible text block.
    QTextBlock first_visible = firstVisibleBlock();
    auto last_visible_line_position = QPoint(0, viewport()->height() - 1);
    QTextBlock last_visible_block = cursorForPosition(last_visible_line_position).block();

    // Get the first and last visible line numbers in the editor.
    int first_visible_line_number = firstVisibleBlock().blockNumber();
    int last_visible_line_number = last_visible_block.blockNumber();

    // Only scroll the textbox if the target line is not currently visible.
    // Scroll 5 lines above the cursor position if the cursor is too close to the editor upper bound.
    bool is_on_creen = (first_visible_line_number < line_number) && (line_number < last_visible_line_number);
    if (!is_on_creen)
    {
        // Compensate for the scrollbar being zero-based, while the incoming line number is one-based.
        int scroll_line = line_number - 1;

        // Scroll to 5 lines above the given line index.
        verticalScrollBar()->setValue(scroll_line - 5);
    }
    else if (line_number - first_visible_line_number < 5)
    {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - 5);
    }
}

void RgSourceCodeEditor::setText(const QString& txt)
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

void RgSourceCodeEditor::clearText(const QString& txt)
{
    QTextDocument* doc = this->document();
    if (doc != nullptr && !doc->isEmpty())
    {
        doc->clear();
    }
}

const std::string& RgSourceCodeEditor::GetTitleBarText()
{
    return title_bar_notification_text_;
}

void RgSourceCodeEditor::SetTitleBarText(const std::string& text)
{
    title_bar_notification_text_ = text;
}

void RgSourceCodeEditor::HandleToggleCursorVisibility()
{
    // Toggle the visibility of the cursor and trigger a repaint.
    is_cursor_visible_ = !is_cursor_visible_;
    viewport()->update();
}

void RgSourceCodeEditor::UpdateLineNumberAreaWidth(int /* new_block_count */)
{
    setViewportMargins(LineNumberAreaWidth(), 0, 0, 0);
}

void RgSourceCodeEditor::UpdateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
    {
        line_number_area_->scroll(0, dy);
    }
    else
    {
        line_number_area_->update(0, rect.y(), line_number_area_->width(), rect.height());
    }

    if (rect.contains(viewport()->rect()))
    {
        UpdateLineNumberAreaWidth(0);
    }
}

void RgSourceCodeEditor::HandleOpenHeaderFile()
{
    QString line_text;
    bool is_valid = GetCurrentLineText(line_text);
    if (is_valid)
    {
        // Parse the line.
        if (IsIncludeDirectiveLine(line_text))
        {
            // Check the type of the include directive: double-quotes or triangular.
            bool is_double_quoted = (line_text.count("\"") == 2);
            bool is_triangular = !is_double_quoted && (line_text.count("<") == 1) && (line_text.count(">") == 1) &&
                (std::find(line_text.begin(), line_text.end(), "<") < std::find(line_text.begin(), line_text.end(), ">"));

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
                        emit OpenHeaderFileRequested(filename);
                    }
                }
            }
        }
    }
}

bool RgSourceCodeEditor::GetCurrentLineText(QString& line_text)
{
    // Get the current line.
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    int lines = 1;
    while (cursor.positionInBlock() > 0)
    {
        cursor.movePosition(QTextCursor::Up);
        lines++;
    }

    QTextBlock block = cursor.block().previous();
    while (block.isValid())
    {
        lines += block.lineCount();
        block = block.previous();
    }

    // Extract the current line's text.
    bool is_valid = GetTextAtLine(lines, line_text) && !line_text.isEmpty();
    return is_valid;
}

bool RgSourceCodeEditor::IsIncludeDirectiveLine(const QString& line_text)
{
    // We use this token to identify if a line is an include directive.
    static const char* INCLUDE_DIR_TOKEN = "#include ";
    bool is_include_directive = line_text.startsWith(INCLUDE_DIR_TOKEN);
    return is_include_directive;
}

void RgSourceCodeEditor::ShowContextMenu(const QPoint& pt)
{
    // Is the open header file action relevant.
    QString line_text;
    bool is_valid = GetCurrentLineText(line_text);

    // Set the open header action to enabled
    // only when the line is an include directive.
    open_header_file_action_->setEnabled(IsIncludeDirectiveLine(line_text));

    // Only enable cut, copy if there is text selected.
    QTextCursor cursor = this->textCursor();
    bool has_selection = cursor.hasSelection();
    cut_text_action_->setEnabled(has_selection);
    copy_text_action_->setEnabled(has_selection);

    // Paste is enabled if there is anything in the clipboard.
    QString clipboard = QApplication::clipboard()->text();
    paste_text_action_->setEnabled(!clipboard.isEmpty());

    // Show the context menu to the user where the mouse is.
    assert(context_menu_ != nullptr);
    if (context_menu_ != nullptr)
    {
        context_menu_->exec(QCursor::pos());
    }
}

void RgSourceCodeEditor::ConnectSignals()
{
    bool is_connected = connect(this, &RgSourceCodeEditor::blockCountChanged, this, &RgSourceCodeEditor::UpdateLineNumberAreaWidth);
    assert(is_connected);

    is_connected = connect(this, &RgSourceCodeEditor::updateRequest, this, &RgSourceCodeEditor::UpdateLineNumberArea);
    assert(is_connected);

    is_connected = connect(this, &RgSourceCodeEditor::cursorPositionChanged, this, &RgSourceCodeEditor::UpdateCursorPosition);
    assert(is_connected);

    is_connected = connect(cursor_blink_timer_, &QTimer::timeout, this, &RgSourceCodeEditor::HandleToggleCursorVisibility);
    assert(is_connected);
}

void RgSourceCodeEditor::HighlightCursorLine(QList<QTextEdit::ExtraSelection>& selections)
{
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection current_line_selection;
        current_line_selection.format.setBackground(kColorHighlightedRow);
        current_line_selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        current_line_selection.cursor = textCursor();
        current_line_selection.cursor.clearSelection();

        // Add the current line's selection to the output list.
        selections.append(current_line_selection);
    }
}

void RgSourceCodeEditor::HighlightCorrelatedSourceLines(QList<QTextEdit::ExtraSelection>& selections)
{
    if (!isReadOnly())
    {
        QTextDocument* file_document = document();
        for (int row_index : highlighted_row_indices_)
        {
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(kColorHighlightedRow);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);

            QTextBlock line_text_block = file_document->findBlockByLineNumber(row_index - 1);
            QTextCursor cursor(line_text_block);
            selection.cursor = cursor;
            selection.cursor.clearSelection();

            // Add the line highlight to the output list.
            selections.append(selection);
        }
    }
}

void RgSourceCodeEditor::SetHighlightedLines(const QList<int>& line_indices)
{
    // Set the list of highlighted lines.
    highlighted_row_indices_ = line_indices;

    // Perform the cursor position change related logic.
    UpdateCursorPositionHelper(true);

    if (!line_indices.isEmpty())
    {
        // Emit a signal indicating that the highlighted line has changed.
        emit SelectedLineChanged(this, line_indices[0]);
    }
}

void RgSourceCodeEditor::UpdateCursorPositionHelper(bool is_correlated)
{
    // A list of highlights to apply to the source editor lines.
    QList<QTextEdit::ExtraSelection> extra_selections;

    if (is_correlated)
    {
        // Automatic correlation: add highlights for the disassembly-correlated source lines.
        HighlightCorrelatedSourceLines(extra_selections);
    }

    // If there aren't any correlated lines to highlight, just highlight the user's currently-selected line.
    if (extra_selections.empty())
    {
        // Standard user selection: add a highlight for the current line.
        HighlightCursorLine(extra_selections);
    }

    // Set the line selections in the source editor.
    setExtraSelections(extra_selections);

    // Track the current and previously-selected line numbers.
    int current_line = GetSelectedLineNumber();
    static int last_cursor_position = current_line;
    if (current_line != last_cursor_position)
    {
        // If the selected line number has changed, emit a signal.
        emit SelectedLineChanged(this, current_line);
        last_cursor_position = current_line;
    }
}

void RgSourceCodeEditor::paintEvent(QPaintEvent* event)
{
    // Invoke the default QPlainTextEdit paint function.
    QPlainTextEdit::paintEvent(event);

    // Paint the cursor manually.
    if (is_cursor_visible_)
    {
        QPainter cursor_painter(viewport());

        // Draw the cursor on top of the text editor.
        QRect cursor_line = cursorRect();
        cursor_line.setWidth(1);
        cursor_painter.fillRect(cursor_line, Qt::SolidPattern);
    }
}

void RgSourceCodeEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    line_number_area_->setGeometry(QRect(cr.left(), cr.top(), LineNumberAreaWidth(), cr.height()));

    emit EditorResized();
}

void RgSourceCodeEditor::hideEvent(QHideEvent* event)
{
    emit EditorHidden();
}

void RgSourceCodeEditor::UpdateCursorPosition()
{
    UpdateCursorPositionHelper(false);
}

bool RgSourceCodeEditor::SetSyntaxHighlighting(RgSrcLanguage lang)
{
    bool result = (lang != RgSrcLanguage::Unknown);
    if (result)
    {
        if (syntax_highlighter_ != nullptr)
        {
            delete syntax_highlighter_;
        }
        syntax_highlighter_ = new RgSyntaxHighlighter(document(), lang);
    }

    return result;
}

int RgSourceCodeEditor::GetSelectedLineNumber() const
{
    return textCursor().blockNumber() + 1;
}

bool RgSourceCodeEditor::GetTextAtLine(int line_number, QString& text) const
{
    bool ret = false;

    bool is_line_valid = line_number >= 1 && line_number < document()->blockCount();
    if (is_line_valid)
    {
        QTextBlock lineBlock = document()->findBlockByLineNumber(line_number - 1);
        text = lineBlock.text();
        ret = true;
    }

    return ret;
}

void RgSourceCodeEditor::LineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(line_number_area_);
    painter.fillRect(event->rect(), QColor(Qt::lightGray).lighter(120));

    QTextBlock block = firstVisibleBlock();
    int block_number = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(block_number + 1);
            painter.setPen(QColor(Qt::darkGray).darker(100));
            QFont default_font = this->document()->defaultFont();
            painter.setFont(default_font);
            painter.drawText(0, top, line_number_area_->width(), fontMetrics().height(),
                Qt::AlignCenter, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++block_number;
    }
}

void RgSourceCodeEditor::mousePressEvent(QMouseEvent* event)
{
    // Close the context menu if it is open.
    if (context_menu_ != nullptr)
    {
        context_menu_->close();
    }

    // Only open the context menu on right-click.
    // In that case do not process the event further.
    if (event != nullptr && event->button() == Qt::RightButton)
    {
        // Simulate a left click event to bring us to the current line.
        Qt::MouseButtons buttons;
        QMouseEvent* dummy_event = new QMouseEvent(event->type(), event->localPos(), event->screenPos(),
            Qt::MouseButton::LeftButton, buttons, event->modifiers());
        emit mousePressEvent(dummy_event);
    }
    else
    {
        // Disable disassembly view's scroll bar signals.
        // This is needed because when the user clicks on source code editor,
        // the disassembly view's scroll bars emit a signal, causing the
        // disassembly view's border to be colored red, and now the user
        // will see both the source code editor and the disassembly view
        // with a red border around it.
        emit DisableScrollbarSignals();

        emit SourceCodeEditorFocusInEvent();

        // Pass the event onto the base class.
        QPlainTextEdit::mousePressEvent(event);

        // Enable disassembly view's scroll bar signals.
        emit EnableScrollbarSignals();
    }
}

void RgSourceCodeEditor::mouseDoubleClickEvent(QMouseEvent* event)
{
    static const QColor kHighlightGreenColor = QColor::fromRgb(124, 252, 0);
    static const QColor kHighlightDarkGreenColor = QColor::fromRgb(0, 102, 51);

    // Override the double-click event to avoid QPlainTextEdit
    // from interpreting the sequence that happens when we simulate
    // a left click as an event of a triple click and select the entire
    // row's text.

    // Get the word under the cursor.
    QTextCursor cursor = this->textCursor();
    cursor.select(QTextCursor::SelectionType::WordUnderCursor);
    this->setTextCursor(cursor);

    // Highlight the current line with yellow background.
    QTextEdit::ExtraSelection current_line_selection;
    current_line_selection.format.setBackground(kColorHighlightedRow);
    current_line_selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    current_line_selection.cursor = textCursor();
    current_line_selection.cursor.clearSelection();

    // Add the current line's selection to the output list.
    QList<QTextEdit::ExtraSelection> extra_selections;
    extra_selections.append(current_line_selection);

    // Setup foreground and background colors to
    // highlight the double clicked word.
    QBrush                           background_brush(kHighlightGreenColor);
    QBrush                           text_brush(Qt::black);
    QPen                             outline_color(Qt::gray, 1);
    QString                          selected_text = cursor.selectedText();

    // Get indices of all matching text.
    std::vector<size_t> search_result_indices;
    RgUtils::FindSearchResultIndices(this->toPlainText(), selected_text, search_result_indices);

    for (auto text_position : search_result_indices)
    {
        QTextEdit::ExtraSelection selected_word;
        selected_word.cursor = QTextCursor(document());
        selected_word.cursor.setPosition(text_position);
        selected_word.cursor.setPosition(text_position + selected_text.length(), QTextCursor::KeepAnchor);
        selected_word.format.setForeground(text_brush);
        selected_word.format.setBackground(background_brush);
        selected_word.format.setProperty(QTextFormat::OutlinePen, outline_color);

        // Save the selection.
        extra_selections.append(selected_word);
    }

    // Highlight the current selection in darker green.
    QPalette palette = this->palette();
    palette.setColor(QPalette::Highlight, kHighlightDarkGreenColor);
    setPalette(palette);

    // Set the selections that were just created.
    setExtraSelections(extra_selections);
}

