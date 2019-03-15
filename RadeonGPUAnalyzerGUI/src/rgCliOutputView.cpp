// C++.
#include <cassert>

// Qt.
#include <QScrollBar>
#include <QTextEdit>
#include <QTextBlock>
#include <QMouseEvent>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgCliOutputView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenu.h>

// Local constants
static const QString  QSTR_CMPLR_ERROR_TOKEN          = ": error:";
static const QString  QSTR_CMPLR_WARNING_TOKEN        = ": warning:";
static const QString  QSTR_CMPLR_NOTE_TOKEN           = ": note:";
static const QChar    QCHAR_CMPLR_ERROR_DELIMITER     = ':';
static const QChar    QCHAR_L_PARENTHESIS             = '(';
static const QChar    QCHAR_R_PARENTHESIS             = ')';

rgCliOutputView::rgCliOutputView(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);

    // Use the output text edit as the focus proxy for this view.
    setFocusProxy(ui.outputTextEdit);

    // To receive focus, the user must tab or click onto the output window widget.
    setFocusPolicy(Qt::StrongFocus);

    // Use monospace font style so that characters align.
    QFont font("unexistent");
    font.setStyleHint(QFont::Monospace);
    ui.outputTextEdit->setFont(font);

    // Set the status and tool tips.
    ui.clearOutputPushButton->setStatusTip(STR_OUTPUT_WINDOW_CLEAR_BUTTON_STATUSTIP);
    ui.clearOutputPushButton->setToolTip(STR_OUTPUT_WINDOW_CLEAR_BUTTON_TOOLTIP);

    // Connect the signals.
    ConnectSignals();

    // Set the event interceptor for TextEdit visible area.
    ui.outputTextEdit->viewport()->installEventFilter(this);

    // Set the mouse cursor to pointing hand cursor.
    SetCursor();

    // Create shortcut actions.
    CreateActions();
}

void rgCliOutputView::CreateActions()
{
    // Tab key navigation.
    m_pTabKeyAction = new QAction(this);
    m_pTabKeyAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_TAB));
    m_pTabKeyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    addAction(m_pTabKeyAction);
    bool isConnected = connect(m_pTabKeyAction, &QAction::triggered, this, &rgCliOutputView::HandleTabFocusPressed);
    assert(isConnected);

    // Shift+Tab key navigation.
    m_pShiftTabKeyAction = new QAction(this);
    m_pShiftTabKeyAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_SHIFT_TAB));
    m_pShiftTabKeyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    addAction(m_pShiftTabKeyAction);
    isConnected = connect(m_pShiftTabKeyAction, &QAction::triggered, this, &rgCliOutputView::HandleShiftTabFocusPressed);
    assert(isConnected);
}

void rgCliOutputView::HandleTabFocusPressed()
{
    // Switch focus to the next child widget,
    // unless the focus is on the last child widget,
    // in which case switch focus to file menu.
    if (m_currentSubWidget == CliOutputWindowSubWidgets::OutputWindow)
    {
        ui.clearOutputPushButton->setFocus();
        m_currentSubWidget = CliOutputWindowSubWidgets::ClearWindowButton;
    }
    else if (m_currentSubWidget == CliOutputWindowSubWidgets::ClearWindowButton)
    {
        m_currentSubWidget = CliOutputWindowSubWidgets::OutputWindow;
        emit FocusNextView();
    }
}

void rgCliOutputView::HandleShiftTabFocusPressed()
{
    // Switch focus to the next child widget,
    // unless the focus is on the last child widget,
    // in which case switch focus to file menu.
    if (m_currentSubWidget == CliOutputWindowSubWidgets::OutputWindow)
    {
        emit FocusColumnPushButton();
    }
    else if (m_currentSubWidget == CliOutputWindowSubWidgets::ClearWindowButton)
    {
        m_currentSubWidget = CliOutputWindowSubWidgets::OutputWindow;
        emit FocusOutputWindow();
    }
}

void rgCliOutputView::mousePressEvent(QMouseEvent *pEvent)
{
    m_currentSubWidget = CliOutputWindowSubWidgets::OutputWindow;

    // Pass the event onto the base class.
    QWidget::mousePressEvent(pEvent);
}

std::string rgCliOutputView::GetText() const
{
    return ui.outputTextEdit->toPlainText().toStdString();
}

void rgCliOutputView::focusInEvent(QFocusEvent* pEvent)
{
    m_currentSubWidget = CliOutputWindowSubWidgets::OutputWindow;
}

void rgCliOutputView::ClearText()
{
    ui.outputTextEdit->clear();
}

bool rgCliOutputView::eventFilter(QObject* pObject, QEvent* pEvent)
{
    assert(pEvent != nullptr);
    if (pEvent != nullptr)
    {
        // Intercept "mouse double-click" events for TextEdit area.
        if (pEvent->type() == QMouseEvent::MouseButtonDblClick)
        {
            const QPoint  clickedPos = static_cast<QMouseEvent*>(pEvent)->pos();
            QTextCursor   textPos = ui.outputTextEdit->cursorForPosition(clickedPos);
            SwitchToErrorLocation(textPos.blockNumber());
        }
        else if (pEvent->type() == QMouseEvent::MouseButtonPress)
        {
            m_currentSubWidget = CliOutputWindowSubWidgets::OutputWindow;
        }
    }

    // Continue default processing for all other events.
    return QObject::eventFilter(pObject, pEvent);
}

void rgCliOutputView::ConnectSignals()
{
    bool isConnected = connect(this, &rgCliOutputView::EmitSetText, this, &rgCliOutputView::HandleAppendText);
    assert(isConnected);

    isConnected = connect(ui.clearOutputPushButton, &QPushButton::clicked, this, &rgCliOutputView::HandleClearClicked);
    assert(isConnected);
}

void rgCliOutputView::ScrollToBottom()
{
    QScrollBar* pScrollBar = ui.outputTextEdit->verticalScrollBar();
    pScrollBar->setValue(pScrollBar->maximum());
}

void rgCliOutputView::HandleAppendText(const QString& str)
{
    ui.outputTextEdit->append(str);
    ScrollToBottom();

    // Focus on the output window when text is appended (this is the ONLY time the window will receive focus).
    setFocus();
}

void rgCliOutputView::HandleClearClicked()
{
    ui.outputTextEdit->clear();
}

void rgCliOutputView::HandleBuildStarted()
{
    // Do not allow to clear the output window during a build.
    ui.clearOutputPushButton->setEnabled(false);
}

void rgCliOutputView::HandleBuildEnded()
{
    // Re-enable the clear button after the build is over.
    ui.clearOutputPushButton->setEnabled(true);
}

void rgCliOutputView::SetCursor()
{
    // Set the mouse cursor to pointing hand cursor.
    ui.clearOutputPushButton->setCursor(Qt::PointingHandCursor);
    ui.viewMaximizeButton->setCursor(Qt::PointingHandCursor);
}

//
// Parse the error location generated by CLI Compiler. Returns a pair {file_name, line}.
//
static bool  ParseErrorLocation(const QStringRef& loc, std::pair<QStringRef&, int&> parsedLoc)
{
    // The clang and lld may produce 2 formats of error location messages:
    // 1.   C:\OpenCL\test.cl:4:7: error: ...
    //      `-- file_name---' ^ ^
    //                  line--' `--column
    //
    // 2.   ...(C:\OpenCL\test.cl:4)...
    //          `-- file_name---' ^
    //                      line--'
    //
    // Note that the source file path may contain symbols lile ':', '(', ')', etc.

    bool  found = false;

    // I. Try matching the 1st pattern.
    int  columnOffset, rParenthOffset = loc.lastIndexOf(QCHAR_R_PARENTHESIS);
    if (rParenthOffset != -1 && (columnOffset = loc.lastIndexOf(QCHAR_CMPLR_ERROR_DELIMITER, rParenthOffset)) != -1)
    {
        // Parse the "loc" string backwards trying to find '(' matching the ')' found at rParenthOffset.
        int  parenthStackCount = 1, offset = rParenthOffset;
        while (offset > 0 && parenthStackCount > 0)
        {
            const QChar sym = loc.at(--offset);
            parenthStackCount += (sym == QCHAR_L_PARENTHESIS ? -1 : (sym == QCHAR_R_PARENTHESIS ? 1 : 0));
        }

        if (parenthStackCount == 0 && columnOffset > offset)
        {
            QStringRef  lineNumStr = loc.mid(columnOffset + 1, rParenthOffset - columnOffset - 1);
            parsedLoc.second = lineNumStr.toInt(&found);
            if (found)
            {
                std::get<0>(parsedLoc) = loc.mid(offset + 1, columnOffset - offset - 1);
            }
        }
    }


    // II. Try mathching the 2nd pattern.
    if (!found)
    {
        int offset, lineStartOffset, lineEndOffset;

        if ((offset = loc.indexOf(QSTR_CMPLR_ERROR_TOKEN)) != -1 ||
            (offset = loc.indexOf(QSTR_CMPLR_WARNING_TOKEN)) != -1 ||
            (offset = loc.indexOf(QSTR_CMPLR_NOTE_TOKEN)) != -1)
        {
            // 1. Skip the column number and parse the line number.
            if ((lineEndOffset = loc.lastIndexOf(QCHAR_CMPLR_ERROR_DELIMITER, offset - 1)) != -1 &&
                (lineStartOffset = loc.lastIndexOf(QCHAR_CMPLR_ERROR_DELIMITER, lineEndOffset - 1)) != -1)
            {
                QStringRef  lineStr = loc.mid(lineStartOffset + 1, lineEndOffset - lineStartOffset - 1);
                std::get<1>(parsedLoc) = lineStr.toInt(&found);
            }

            // 2. Copy the file name.
            if (found)
            {
                std::get<0>(parsedLoc) = loc.left(lineStartOffset);
            }
        }
    }

    return found;
}

void rgCliOutputView::SwitchToErrorLocation(int blockNum) const
{
    if (blockNum > 0)
    {
        if (QTextDocument* doc = ui.outputTextEdit->document())
        {
            QTextBlock    block = doc->findBlockByNumber(blockNum);
            QString       blockText = block.text();
            QStringRef    filePath;
            int           line;

            if (!blockText.isEmpty() && ParseErrorLocation(QStringRef(&blockText), { filePath, line }))
            {
                emit SwitchToFile(filePath.toString().toStdString(), line);
            }
        }
    }
}
