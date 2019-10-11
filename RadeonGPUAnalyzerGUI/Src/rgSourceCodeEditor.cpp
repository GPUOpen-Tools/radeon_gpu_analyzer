// C++.
#include <cassert>

// Qt.
#include <QtWidgets>
#include <QMenu>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>

// Use a light yellow to highlight the line that the cursor is on.
static const QColor COLOR_HIGHLIGTED_ROW = QColor(Qt::yellow).lighter(170);

rgSourceCodeEditor::rgSourceCodeEditor(QWidget* pParent, rgSrcLanguage lang) : QPlainTextEdit(pParent)
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
    if (lang != rgSrcLanguage::Unknown)
    {
        m_pSyntaxHighlighter = new rgSyntaxHighlighter(document(), lang);
    }

    UpdateLineNumberAreaWidth(0);

    // Initialize rendering of highlighted lines within the editor.
    UpdateCursorPosition();

    // Set the default font.
    QTextDocument* pDoc = this->document();
    if (pDoc != nullptr)
    {
        QFont font = pDoc->defaultFont();
        font.setFamily(STR_BUILD_VIEW_FONT_FAMILY);
        font.setPointSize(gs_BUILD_VIEW_FONT_SIZE);
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

    // Set up the open header file action.
    m_pOpenHeaderFileAction = new QAction(tr(STR_SOURCE_EDITOR_CONTEXT_MENU_OPEN_HEADER), this);

    // Cut action.
    m_pCutTextAction = new QAction(tr(STR_SOURCE_EDITOR_CONTEXT_MENU_CUT), this);
    m_pCutTextAction->setShortcut(QKeySequence(gs_SOURCE_EDITOR_HOTKEY_CONTEXT_MENU_CUT));

    // Copy action.
    m_pCopyTextAction = new QAction(tr(STR_SOURCE_EDITOR_CONTEXT_MENU_COPY), this);
    m_pCopyTextAction->setShortcut(QKeySequence(gs_SOURCE_EDITOR_HOTKEY_CONTEXT_MENU_COPY));

    // Paste action.
    m_pPasteTextAction = new QAction(tr(STR_SOURCE_EDITOR_CONTEXT_MENU_PASTE), this);
    m_pPasteTextAction->setShortcut(QKeySequence(gs_SOURCE_EDITOR_HOTKEY_CONTEXT_MENU_PASTE));

    // Select All action.
    m_pSelectAllTextAction = new QAction(tr(STR_SOURCE_EDITOR_CONTEXT_MENU_SELECT_ALL), this);
    m_pCutTextAction->setShortcut(QKeySequence(gs_SOURCE_EDITOR_HOTKEY_CONTEXT_MENU_SELECT_ALL));

    // Open header file.
    bool isConnected = connect(m_pOpenHeaderFileAction, &QAction::triggered, this, &rgSourceCodeEditor::HandleOpenHeaderFile);
    assert(isConnected);

    // Cut action.
    isConnected = connect(m_pCutTextAction, &QAction::triggered, this, &QPlainTextEdit::cut);
    assert(isConnected);

    // Copy action.
    isConnected = connect(m_pCopyTextAction, &QAction::triggered, this, &QPlainTextEdit::copy);
    assert(isConnected);

    // Paste action.
    isConnected = connect(m_pPasteTextAction, &QAction::triggered, this, &QPlainTextEdit::paste);
    assert(isConnected);

    // Select All action.
    isConnected = connect(m_pSelectAllTextAction, &QAction::triggered, this, &QPlainTextEdit::selectAll);
    assert(isConnected);

    m_pContextMenu = this->createStandardContextMenu();
    assert(m_pContextMenu != nullptr);
    if (m_pContextMenu != nullptr)
    {
        // Reconstruct the context menu.
        m_pContextMenu->clear();
        m_pContextMenu->addAction(m_pOpenHeaderFileAction);
        m_pContextMenu->addSeparator();
        m_pContextMenu->addAction(m_pCutTextAction);
        m_pContextMenu->addAction(m_pCopyTextAction);
        m_pContextMenu->addAction(m_pPasteTextAction);
        m_pContextMenu->addSeparator();
        m_pContextMenu->addAction(m_pSelectAllTextAction);
        this->setContextMenuPolicy(Qt::CustomContextMenu);

        // Set hand pointer for the context menu.
        m_pContextMenu->setCursor(Qt::PointingHandCursor);

        // Connect the signal for showing the context menu.
        isConnected = connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
        assert(isConnected);
    }

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

const std::string& rgSourceCodeEditor::GetTitleBarText()
{
    return m_titleBarNotificationText;
}

void rgSourceCodeEditor::SetTitleBarText(const std::string& text)
{
    m_titleBarNotificationText = text;
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

void rgSourceCodeEditor::HandleOpenHeaderFile()
{
    QString lineText;
    bool isValid = GetCurrentLineText(lineText);
    if (isValid)
    {
        // Parse the line.
        if (IsIncludeDirectiveLine(lineText))
        {
            // Check the type of the include directive: double-quotes or triangular.
            bool isDoubleQuoted = (lineText.count("\"") == 2);
            bool isTriangular = !isDoubleQuoted && (lineText.count("<") == 1) && (lineText.count(">") == 1) &&
                (std::find(lineText.begin(), lineText.end(), "<") < std::find(lineText.begin(), lineText.end(), ">"));

            if (isDoubleQuoted)
            {
                // Extract the file name.
                QStringList lineBroken = lineText.split("\"");
                if (lineBroken.size() >= 2)
                {
                    QString fileName = lineBroken[1];

                    // Fire the signal: user requested to open header file.
                    emit OpenHeaderFileRequested(fileName);
                }
            }
            else if (isTriangular)
            {
                // Extract the part that is after the first < character.
                QStringList lineBrokenA = lineText.split("<");
                if (lineBrokenA.size() >= 2)
                {
                    // Extract the part that is before the > character.
                    QStringList lineBrokenB = lineBrokenA.at(1).split(">");
                    if (lineBrokenB.size() >= 1)
                    {
                        // Fire the signal: user requested to open header file.
                        QString fileName = lineBrokenB[0];
                        emit OpenHeaderFileRequested(fileName);
                    }
                }
            }
        }
    }
}

bool rgSourceCodeEditor::GetCurrentLineText(QString& lineText)
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
    bool isValid = GetTextAtLine(lines, lineText) && !lineText.isEmpty();
    return isValid;
}

bool rgSourceCodeEditor::IsIncludeDirectiveLine(const QString& lineTxt)
{
    // We use this token to identify if a line is an include directive.
    static const char* INCLUDE_DIR_TOKEN = "#include ";
    bool isIncludeDirective = lineTxt.startsWith(INCLUDE_DIR_TOKEN);
    return isIncludeDirective;
}

void rgSourceCodeEditor::ShowContextMenu(const QPoint& pt)
{
    // Is the open header file action relevant.
    QString lineText;
    bool isValid = GetCurrentLineText(lineText);

    // Set the open header action to enabled
    // only when the line is an include directive.
    m_pOpenHeaderFileAction->setEnabled(IsIncludeDirectiveLine(lineText));

    // Only enable cut, copy if there is text selected.
    QTextCursor cursor = this->textCursor();
    bool hasSelection = cursor.hasSelection();
    m_pCutTextAction->setEnabled(hasSelection);
    m_pCopyTextAction->setEnabled(hasSelection);

    // Paste is enabled if there is anything in the clipboard.
    QString clipboard = QApplication::clipboard()->text();
    m_pPasteTextAction->setEnabled(!clipboard.isEmpty());

    // Show the context menu to the user where the mouse is.
    assert(m_pContextMenu != nullptr);
    if (m_pContextMenu != nullptr)
    {
        m_pContextMenu->exec(QCursor::pos());
    }
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

bool rgSourceCodeEditor::SetSyntaxHighlighting(rgSrcLanguage lang)
{
    bool result = (lang != rgSrcLanguage::Unknown);
    if (result)
    {
        if (m_pSyntaxHighlighter != nullptr)
        {
            delete m_pSyntaxHighlighter;
        }
        m_pSyntaxHighlighter = new rgSyntaxHighlighter(document(), lang);
    }

    return result;
}

int rgSourceCodeEditor::GetSelectedLineNumber() const
{
    return textCursor().blockNumber() + 1;
}

bool rgSourceCodeEditor::GetTextAtLine(int lineNumber, QString& text) const
{
    bool ret = false;

    bool isLineValid = lineNumber >= 1 && lineNumber < document()->blockCount();
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
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();

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
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void rgSourceCodeEditor::mousePressEvent(QMouseEvent* pEvent)
{
    // Close the context menu if it is open.
    if (m_pContextMenu != nullptr)
    {
        m_pContextMenu->close();
    }

    // Only open the context menu on right-click.
    // In that case do not process the event further.
    if (pEvent != nullptr && pEvent->button() == Qt::RightButton)
    {
        // Simulate a left click event to bring us to the current line.
        Qt::MouseButtons buttons;
        QMouseEvent* pDummyEvent = new QMouseEvent(pEvent->type(), pEvent->localPos(), pEvent->screenPos(),
            Qt::MouseButton::LeftButton, buttons, pEvent->modifiers());
        emit mousePressEvent(pDummyEvent);
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
        QPlainTextEdit::mousePressEvent(pEvent);

        // Enable disassembly view's scroll bar signals.
        emit EnableScrollbarSignals();
    }
}

void rgSourceCodeEditor::mouseDoubleClickEvent(QMouseEvent * pEvent)
{
    // Override the double-click event to avoid QPlainTextEdit
    // from interpreting the sequence that happens when we simulate
    // a left click as an event of a triple click and select the entire
    // row's text.
    QTextCursor cursor = this->textCursor();
    cursor.select(QTextCursor::SelectionType::WordUnderCursor);
    this->setTextCursor(cursor);
}
