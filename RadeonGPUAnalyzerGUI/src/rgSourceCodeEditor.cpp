// C++.
#include <cassert>

// Qt.
#include <QtWidgets>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgSourceCodeEditor.h>

// Use a light yellow to highlight the line that the cursor is on.
static const QColor COLOR_HIGHLIGTED_ROW = QColor(Qt::yellow).lighter(170);

rgSourceCodeEditor::rgSourceCodeEditor(QWidget* pParent) : QPlainTextEdit(pParent)
{
    m_pLineNumberArea = new LineNumberArea(this);

    // The duration of time between the text cursor blinking on and off.
    static const int gs_CURSOR_BLINK_TOGGLE_DURATION_MS = 500;

    // Create the blinking cursor update timer.
    m_pCursorBlinkTimer = new QTimer(this);
    m_pCursorBlinkTimer->setInterval(gs_CURSOR_BLINK_TOGGLE_DURATION_MS);
    m_pCursorBlinkTimer->start();

    // Connect signals.
    ConnectSignals();

    // Create the syntax highlighter.
    m_pSyntaxHighlight = new rgSyntaxHighlight(document(), rgSyntaxHighlight::Language::OpenCL);

    UpdateLineNumberAreaWidth(0);

    // Initialize rendering of highlighted lines within the editor.
    UpdateCursorPosition();

    // Set the default font.
    QTextDocument* pDoc = this->document();
    if (pDoc != nullptr)
    {
        QFont font = pDoc->defaultFont();
        font.setFamily("Consolas");
        font.setPointSize(10);
        pDoc->setDefaultFont(font);

        // A tab is the same width as 4 spaces.
        QFontMetrics metrics(font);
        int tabWidth = metrics.width(' ') * 4;
        setTabStopWidth(tabWidth);
    }

    // Configure the word wrap mode.
    setWordWrapMode(QTextOption::NoWrap);

    // Disable accepting dropped files.
    setAcceptDrops(false);
}

int rgSourceCodeEditor::LineNumberAreaWidth() const
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

void rgSourceCodeEditor::ScrollToLine(int lineNumber)
{
    // Compute the last visible text block.
    QTextBlock firstVisible = firstVisibleBlock();
    auto lastVisibleLinePosition = QPoint(0, viewport()->height() - 1);
    QTextBlock lastVisibleBlock = cursorForPosition(lastVisibleLinePosition).block();

    // Get the first and last visible line numbers in the editor.
    int firstVisibleLineNumber = firstVisibleBlock().blockNumber();
    int lastVisibleLineNumber = lastVisibleBlock.blockNumber();

    // Only scroll the textbox if the target line is not currently visible.
    // Scroll 5 lines above the cursor position if the cursor is too close to the editor upper bound.
    bool isOnscreen = (firstVisibleLineNumber < lineNumber) && (lineNumber < lastVisibleLineNumber);
    if (!isOnscreen)
    {
        // Compensate for the scrollbar being zero-based, while the incoming line number is one-based.
        int scrollLine = lineNumber - 1;

        // Scroll to 5 lines above the given line index.
        verticalScrollBar()->setValue(scrollLine - 5);
    }
    else if (lineNumber - firstVisibleLineNumber < 5)
    {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - 5);
    }
}

void rgSourceCodeEditor::setText(const QString& txt)
{
    QTextDocument* pDoc = this->document();
    if (pDoc != nullptr)
    {
        // Set the text.
        pDoc->setPlainText(txt);

        // Set the cursor to the first line and column.
        this->moveCursor(QTextCursor::Start);
        this->ensureCursorVisible();
    }
}

void rgSourceCodeEditor::clearText(const QString& txt)
{
    QTextDocument* pDoc = this->document();
    if (pDoc != nullptr && !pDoc->isEmpty())
    {
        pDoc->clear();
    }
}

void rgSourceCodeEditor::HandleToggleCursorVisibility()
{
    // Toggle the visibility of the cursor and trigger a repaint.
    m_isCursorVisible = !m_isCursorVisible;
    viewport()->update();
}

void rgSourceCodeEditor::UpdateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(LineNumberAreaWidth(), 0, 0, 0);
}

void rgSourceCodeEditor::UpdateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_pLineNumberArea->scroll(0, dy);
    else
        m_pLineNumberArea->update(0, rect.y(), m_pLineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        UpdateLineNumberAreaWidth(0);
}

void rgSourceCodeEditor::ConnectSignals()
{
    bool isConnected = connect(this, &rgSourceCodeEditor::blockCountChanged, this, &rgSourceCodeEditor::UpdateLineNumberAreaWidth);
    assert(isConnected);

    isConnected = connect(this, &rgSourceCodeEditor::updateRequest, this, &rgSourceCodeEditor::UpdateLineNumberArea);
    assert(isConnected);

    isConnected = connect(this, &rgSourceCodeEditor::cursorPositionChanged, this, &rgSourceCodeEditor::UpdateCursorPosition);
    assert(isConnected);

    isConnected = connect(m_pCursorBlinkTimer, &QTimer::timeout, this, &rgSourceCodeEditor::HandleToggleCursorVisibility);
    assert(isConnected);
}

void rgSourceCodeEditor::HighlightCursorLine(QList<QTextEdit::ExtraSelection>& selections)
{
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection currentLineSelection;
        currentLineSelection.format.setBackground(COLOR_HIGHLIGTED_ROW);
        currentLineSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
        currentLineSelection.cursor = textCursor();
        currentLineSelection.cursor.clearSelection();

        // Add the current line's selection to the output list.
        selections.append(currentLineSelection);
    }
}

void rgSourceCodeEditor::HighlightCorrelatedSourceLines(QList<QTextEdit::ExtraSelection>& selections)
{
    if (!isReadOnly())
    {
        QTextDocument* pFileDocument = document();
        for (int rowIndex : m_highlightedRowIndices)
        {
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(COLOR_HIGHLIGTED_ROW);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);

            QTextBlock lineTextBlock = pFileDocument->findBlockByLineNumber(rowIndex - 1);
            QTextCursor cursor(lineTextBlock);
            selection.cursor = cursor;
            selection.cursor.clearSelection();

            // Add the line highlight to the output list.
            selections.append(selection);
        }
    }
}

void rgSourceCodeEditor::SetHighlightedLines(const QList<int>& lineIndices)
{
    // Set the list of highlighted lines.
    m_highlightedRowIndices = lineIndices;

    // Perform the cursor position change related logic.
    UpdateCursorPositionHelper(true);

    if (!lineIndices.isEmpty())
    {
        // Emit a signal indicating that the highlighted line has changed.
        emit SelectedLineChanged(this, lineIndices[0]);
    }
}

void rgSourceCodeEditor::UpdateCursorPositionHelper(bool isCorrelated)
{
    // A list of highlights to apply to the source editor lines.
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (isCorrelated)
    {
        // Automatic correlation: add highlights for the disassembly-correlated source lines.
        HighlightCorrelatedSourceLines(extraSelections);
    }

    // If there aren't any correlated lines to highlight, just highlight the user's currently-selected line.
    if (extraSelections.empty())
    {
        // Standard user selection: add a highlight for the current line.
        HighlightCursorLine(extraSelections);
    }

    // Set the line selections in the source editor.
    setExtraSelections(extraSelections);

    // Track the current and previously-selected line numbers.
    int currentLine = GetSelectedLineNumber();
    static int lastCursorPosition = currentLine;
    if (currentLine != lastCursorPosition)
    {
        // If the selected line number has changed, emit a signal.
        emit SelectedLineChanged(this, currentLine);
        lastCursorPosition = currentLine;
    }
}

void rgSourceCodeEditor::paintEvent(QPaintEvent* pEvent)
{
    // Invoke the default QPlainTextEdit paint function.
    QPlainTextEdit::paintEvent(pEvent);

    // Paint the cursor manually.
    if (m_isCursorVisible)
    {
        QPainter cursorPainter(viewport());

        // Draw the cursor on top of the text editor.
        QRect cursorLine = cursorRect();
        cursorLine.setWidth(1);
        cursorPainter.fillRect(cursorLine, Qt::SolidPattern);
    }
}

void rgSourceCodeEditor::resizeEvent(QResizeEvent* pEvent)
{
    QPlainTextEdit::resizeEvent(pEvent);

    QRect cr = contentsRect();
    m_pLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), LineNumberAreaWidth(), cr.height()));

    emit EditorResized();
}

void rgSourceCodeEditor::hideEvent(QHideEvent* pEvent)
{
    emit EditorHidden();
}

void rgSourceCodeEditor::UpdateCursorPosition()
{
    UpdateCursorPositionHelper(false);
}

int rgSourceCodeEditor::GetSelectedLineNumber() const
{
    return textCursor().blockNumber() + 1;
}

bool rgSourceCodeEditor::GetTextAtLine(int lineNumber, QString& text) const
{
    bool ret = false;

    bool isLineValid = lineNumber >= 1 && lineNumber < document()->blockCount();
    assert(isLineValid);
    if (isLineValid)
    {
        QTextBlock lineBlock = document()->findBlockByLineNumber(lineNumber - 1);
        text = lineBlock.text();
        ret = true;
    }

    return ret;
}

void rgSourceCodeEditor::LineNumberAreaPaintEvent(QPaintEvent* pEvent)
{
    QPainter painter(m_pLineNumberArea);
    painter.fillRect(pEvent->rect(), QColor(Qt::lightGray).lighter(120));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= pEvent->rect().bottom())
    {
        if (block.isVisible() && bottom >= pEvent->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(Qt::darkGray).darker(100));
            QFont defaultFont = this->document()->defaultFont();
            painter.setFont(defaultFont);
            painter.drawText(0, top, m_pLineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignCenter, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}