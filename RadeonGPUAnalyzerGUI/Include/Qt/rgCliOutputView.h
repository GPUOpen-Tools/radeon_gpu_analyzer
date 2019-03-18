#pragma once

// Qt.
#include <QWidget>

// Local.
#include <ui_rgCliOutputView.h>

// Forward declarations.
namespace Ui
{
    class rgCliOutputView;
}

// Indices for sub widgets.
enum class CliOutputWindowSubWidgets
{
    OutputWindow,
    ClearWindowButton,
    Count
};

// A widget embedded in the rgBuildView, used to display CLI invocation output.
class rgCliOutputView : public QWidget
{
    Q_OBJECT

public:
    explicit rgCliOutputView(QWidget* pParent = nullptr);
    ~rgCliOutputView() = default;

    // Clears the output pane text.
    void ClearText();

    // Redefined event filter.
    // Used to intercept events going to TextEdit sub-widget.
    virtual bool eventFilter(QObject* obj, QEvent* event) override;

    // Reimplement focus in event.
    virtual void focusInEvent(QFocusEvent* pEvent) override;

    // Reimplement mouse press event.
    virtual void mousePressEvent(QMouseEvent *pEvent) override;

    // Returns the text of the CLI build output window.
    std::string GetText() const;

signals:
    // A thread-safe signal used to send new text to the output window.
    void EmitSetText(const QString& str);

    // Focus the next view.
    void FocusNextView();

    // Focus output window.
    void FocusOutputWindow();

    // Focus column push button in disassembly view.
    void FocusColumnPushButton();

    // A signal used by rgCliOutputView to make File Menu and Main Menu to switch to required file/line.
    void SwitchToFile(const std::string& filePath, int lineNum) const;

public slots:
    // Handler for build start event.
    void HandleBuildStarted();

    // Handler for build end event.
    void HandleBuildEnded();

private slots:
    // Append new text into the output window.
    void HandleAppendText(const QString& str);

    // Handler invoked when the user clicks the "Clear" button.
    void HandleClearClicked();

    // Handler invoked when the user hits the tab button.
    void HandleTabFocusPressed();

    // Handler invoked when the user hits the shift+tab buttons.
    void HandleShiftTabFocusPressed();

private:
    // Connect widget signals to slots.
    void ConnectSignals();

    // Create shortcut actions.
    void CreateActions();

    // Scroll the output window to the latest text at the bottom.
    void ScrollToBottom();

    // Set the mouse cursor to pointing hand cursor.
    void SetCursor();

    // Parse the specified line in the CLI output text and try to extract the source file name and error line number.
    // If succeeded, ask Source Editor to switch to found file/line.
     void SwitchToErrorLocation(int blockNum) const;

private:
    // The tab key action.
    QAction* m_pTabKeyAction = nullptr;

    // The shift+tab key action.
    QAction* m_pShiftTabKeyAction = nullptr;

    // Keep track of the current sub widget.
    CliOutputWindowSubWidgets m_currentSubWidget = CliOutputWindowSubWidgets::OutputWindow;

    Ui::rgCliOutputView ui;
};