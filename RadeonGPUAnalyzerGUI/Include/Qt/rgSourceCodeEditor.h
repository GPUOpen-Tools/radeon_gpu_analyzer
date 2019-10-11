#pragma once

// Qt.
#include <QPlainTextEdit>
#include <QObject>
#include <QAction>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSyntaxHighlighter.h>

// Forward declarations.
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class LineNumberArea;

class rgSourceCodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    // Ctor
    rgSourceCodeEditor(QWidget* pParent = nullptr, rgSrcLanguage lang = rgSrcLanguage::Unknown);

    // Set required syntax highlighting.
    bool SetSyntaxHighlighting(rgSrcLanguage lang);

    // Get the selected line number that the cursor is on.
    int GetSelectedLineNumber() const;

    // Retrieve the text at the specified line.
    bool GetTextAtLine(int lineNumber, QString& text) const;

    // A callback used to paint the line number area.
    void LineNumberAreaPaintEvent(QPaintEvent* pEvent);

    // Compute the width of the line number area.
    int LineNumberAreaWidth() const;

    // Scroll the editor to the given line number.
    void ScrollToLine(int lineNumber);

    // Sets the contents of the source code view.
    void setText(const QString& txt);

    // Clears the contents of the source code view.
    void clearText(const QString& txt);

    // Get the text of title bar notification currently set for this editor.
    const std::string& GetTitleBarText();

    // Set the text of title bar notification for this editor.
    void SetTitleBarText(const std::string& text);

signals:
    // A signal emitted when the source editor is hidden.
    void EditorHidden();

    // A signal emitted when the source editor is resized.
    void EditorResized();

    // A signal emitted when the user changes the selected line index.
    void SelectedLineChanged(rgSourceCodeEditor* pEditor, int lineIndex);

    // A signal emitted when the source code editor gets the focus.
    void SourceCodeEditorFocusInEvent();

    // A signal to disable table scroll bar signals.
    void DisableScrollbarSignals();

    // A signal to enable table scroll bar signals.
    void EnableScrollbarSignals();

    // A signal that requests the system to open a header file.
    void OpenHeaderFileRequested(const QString& headerFilePath);

public slots:
    // Apply a colored highlight to the background of each given row.
    void SetHighlightedLines(const QList<int>& lineIndices);

protected:
    // An overridden paint handler responsible for painting a blinking cursor when the editor doesn't have focus.
    virtual void paintEvent(QPaintEvent* pEvent) override;

    // An overridden resize handler responsible for recomputing editor geometry.
    virtual void resizeEvent(QResizeEvent* pEvent) override;

    // An overridden "widget was hidden" handler used to emit a signal indicating a visibility change.
    virtual void hideEvent(QHideEvent* pEvent) override;

    // The overridden mousePressEvent.
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

    // Mouse double-click event.
    virtual void mouseDoubleClickEvent(QMouseEvent* pEvent) override;

private slots:
    // A handler invoked when the blinking cursor toggled between visible and hidden.
    void HandleToggleCursorVisibility();

    // A handler invoked when the line number area width is changed.
    void UpdateLineNumberAreaWidth(int newBlockCount);

    // A handler invoked when the cursor has been moved within the document.
    void UpdateCursorPosition();

    // A handler invoked when the line number area is updated.
    void UpdateLineNumberArea(const QRect &, int);

    // Handler for opening a header file.
    void HandleOpenHeaderFile();

    // Get the text of the current line.
    bool GetCurrentLineText(QString& lineTxt);

    // Returns true if the given text represents an
    // include directive, returns false otherwise.
    bool IsIncludeDirectiveLine(const QString& lineText);

    // Show this source view's context menu to the user.
    void ShowContextMenu(const QPoint& location);

protected:
    // Connect the editor signals.
    void ConnectSignals();

    // Used to append a row highlight selection for the current line.
    void HighlightCursorLine(QList<QTextEdit::ExtraSelection>& selections);

    // Used to append row highlights for correlated source code lines.
    void HighlightCorrelatedSourceLines(QList<QTextEdit::ExtraSelection>& selections);

    // Helper function that performs the logic which is related to a cursor position update
    // event, while taking into account the type of the event: correlation vs. user selection.
    void UpdateCursorPositionHelper(bool isCorrelated);

    // The flag indicating if the cursor is visible or hidden.
    bool m_isCursorVisible = true;

    // A timer used to toggle the visibility of the blinking cursor.
    QTimer* m_pCursorBlinkTimer;

    // The list of rows painted with the highlight color.
    QList<int> m_highlightedRowIndices;

    // The line number display widget.
    QWidget* m_pLineNumberArea = nullptr;

    // The syntax highlighter used to alter the rendering of keywords in the source editor.
    rgSyntaxHighlighter* m_pSyntaxHighlighter = nullptr;

    // The notification text shown in the Code Editor title bar.
    std::string m_titleBarNotificationText;

    // Action for opening a header file.
    QAction* m_pOpenHeaderFileAction = nullptr;

    // Action for cutting text.
    QAction* m_pCutTextAction = nullptr;

    // Action for copying text.
    QAction* m_pCopyTextAction = nullptr;

    // Action for pasting text.
    QAction* m_pPasteTextAction = nullptr;

    // Action for selecting all text.
    QAction* m_pSelectAllTextAction = nullptr;

    // This source view's context menu.
    QMenu* m_pContextMenu = nullptr;
};

// A widget used to paint the line number gutter in the source editor.
class LineNumberArea : public QWidget
{
public:
    LineNumberArea(rgSourceCodeEditor* pEditor) : QWidget(pEditor), m_pCodeEditor(pEditor) {}

    // Override the sizeHint based on the line number gutter width.
    virtual QSize sizeHint() const override
    {
        return QSize(m_pCodeEditor->LineNumberAreaWidth(), 0);
    }

protected:
    // Overridden paint for drawing the gutter with line numbers.
    virtual void paintEvent(QPaintEvent* pEvent) override
    {
        m_pCodeEditor->LineNumberAreaPaintEvent(pEvent);
    }

    // Overridden double click event for the line number gutter area.
    virtual void mouseDoubleClickEvent(QMouseEvent* pEvent) override
    {}

private:
    // The editor where line numbers will be painted.
    rgSourceCodeEditor* m_pCodeEditor = nullptr;
};