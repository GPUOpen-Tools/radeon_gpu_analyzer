//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for source code editor.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_CODE_EDITOR_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_CODE_EDITOR_H_

// Qt.
#include <QPlainTextEdit>
#include <QObject>
#include <QAction>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_syntax_highlighter.h"

// Forward declarations.
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class LineNumberArea;

class RgSourceCodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    // Ctor
    RgSourceCodeEditor(QWidget* parent = nullptr, RgSrcLanguage lang = RgSrcLanguage::Unknown);

    // Set required syntax highlighting.
    bool SetSyntaxHighlighting(RgSrcLanguage lang);

    // Get the selected line number that the cursor is on.
    int GetSelectedLineNumber() const;

    // Retrieve the text at the specified line.
    bool GetTextAtLine(int line_number, QString& text) const;

    // A callback used to paint the line number area.
    void LineNumberAreaPaintEvent(QPaintEvent* event);

    // Compute the width of the line number area.
    int LineNumberAreaWidth() const;

    // Scroll the editor to the given line number.
    void ScrollToLine(int line_number);

    // Sets the contents of the source code view.
    void setText(const QString& txt);

    // Clears the contents of the source code view.
    void clearText(const QString& txt);

    // Get the text of title bar notification currently set for this editor.
    const std::string& GetTitleBarText();

    // Set the text of title bar notification for this editor.
    void SetTitleBarText(const std::string& text);

    // Lines in the source code were highlighted. Apply a colored highlight to the background of each given row and notify the disassembly view.
    void SetHighlightedLines(const QList<int>& line_indices);

signals:
    // A signal emitted when the source editor is hidden.
    void EditorHidden();

    // A signal emitted when the source editor is resized.
    void EditorResized();

    // A signal emitted when the user changes the selected line index.
    void SelectedLineChanged(RgSourceCodeEditor* editor, int line_index);

    // A signal emitted when the source code editor gets the focus.
    void SourceCodeEditorFocusInEvent();

    // A signal to disable table scroll bar signals.
    void DisableScrollbarSignals();

    // A signal to enable table scroll bar signals.
    void EnableScrollbarSignals();

    // A signal that requests the system to open a header file.
    void OpenHeaderFileRequested(const QString& header_file_path);

public slots:
    // Lines in the disassembly view were highlighted. Apply a colored highlight to the background of each given row in the source code. Don't notify the disassembly view.
    void HandleHighlightedLinesSet(const QList<int>& line_indices);

protected:
    // An overridden paint handler responsible for painting a blinking cursor when the editor doesn't have focus.
    virtual void paintEvent(QPaintEvent* event) override;

    // An overridden resize handler responsible for recomputing editor geometry.
    virtual void resizeEvent(QResizeEvent* event) override;

    // An overridden "widget was hidden" handler used to emit a signal indicating a visibility change.
    virtual void hideEvent(QHideEvent* event) override;

    // The overridden mousePressEvent.
    virtual void mousePressEvent(QMouseEvent* event) override;

    // Mouse double-click event.
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    // A handler invoked when the blinking cursor toggled between visible and hidden.
    void HandleToggleCursorVisibility();

    // A handler invoked when the line number area width is changed.
    void UpdateLineNumberAreaWidth(int new_block_count);

    // A handler invoked when the cursor has been moved within the document.
    void UpdateCursorPosition();

    // A handler invoked when the line number area is updated.
    void UpdateLineNumberArea(const QRect &, int);

    // Handler for opening a header file.
    void HandleOpenHeaderFile();

    // Get the text of the current line.
    bool GetCurrentLineText(QString& line_txt);

    // Returns true if the given text represents an
    // include directive, returns false otherwise.
    bool IsIncludeDirectiveLine(const QString& line_text);

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
    void UpdateCursorPositionHelper(bool is_correlated);

    // The flag indicating if the cursor is visible or hidden.
    bool is_cursor_visible_ = true;

    // A timer used to toggle the visibility of the blinking cursor.
    QTimer* cursor_blink_timer_;

    // The list of rows painted with the highlight color.
    QList<int> highlighted_row_indices_;

    // The line number display widget.
    QWidget* line_number_area_ = nullptr;

    // The syntax highlighter used to alter the rendering of keywords in the source editor.
    RgSyntaxHighlighter* syntax_highlighter_ = nullptr;

    // The notification text shown in the Code Editor title bar.
    std::string title_bar_notification_text_;

    // Action for opening a header file.
    QAction* open_header_file_action_ = nullptr;

    // Action for cutting text.
    QAction* cut_text_action_ = nullptr;

    // Action for copying text.
    QAction* copy_text_action_ = nullptr;

    // Action for pasting text.
    QAction* paste_text_action_ = nullptr;

    // Action for selecting all text.
    QAction* select_all_text_action_ = nullptr;

    // This source view's context menu.
    QMenu* context_menu_ = nullptr;
};

// A widget used to paint the line number gutter in the source editor.
class LineNumberArea : public QWidget
{
public:
    LineNumberArea(RgSourceCodeEditor* editor) : QWidget(editor), code_editor_(editor) {}

    // Override the sizeHint based on the line number gutter width.
    virtual QSize sizeHint() const override
    {
        return QSize(code_editor_->LineNumberAreaWidth(), 0);
    }

protected:
    // Overridden paint for drawing the gutter with line numbers.
    virtual void paintEvent(QPaintEvent* event) override
    {
        code_editor_->LineNumberAreaPaintEvent(event);
    }

    // Overridden double click event for the line number gutter area.
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        Q_UNUSED(event);
    }

private:
    // The editor where line numbers will be painted.
    RgSourceCodeEditor* code_editor_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_CODE_EDITOR_H_
