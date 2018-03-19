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

signals:
    // A thread-safe signal used to send new text to the output window.
    void EmitSetText(const QString& str);

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

private:
    // Connect widget signals to slots.
    void ConnectSignals();

    // Scroll the output window to the latest text at the bottom.
    void ScrollToBottom();

    // Set the mouse cursor to pointing hand cursor.
    void SetCursor();

    // Parse the specified line in the CLI output text and try to extract the source file name and error line number.
    // If succeeded, ask Source Editor to switch to found file/line.
     void SwitchToErrorLocation(int blockNum) const;

private:
    Ui::rgCliOutputView ui;
};