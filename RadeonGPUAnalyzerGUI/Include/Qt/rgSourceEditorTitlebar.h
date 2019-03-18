#pragma once

// Qt.
#include <QFrame>

// Local.
#include "ui_rgSourceEditorTitlebar.h"

// A title bar inserted above the source editor that displays the correlation status.
class rgSourceEditorTitlebar : public QFrame
{
    Q_OBJECT

public:
    rgSourceEditorTitlebar(QWidget* pParent = nullptr);
    virtual ~rgSourceEditorTitlebar() = default;

    // Update the title bar view based on whether or not a file's correlation state is enabled.
    void SetIsCorrelationEnabled(bool isEnabled);

    // Show the required message in the titlebar.
    void ShowMessage(const std::string& msg);

    // Change the visibility of the title bar contents.
    void SetTitlebarContentsVisibility(bool isVisible);

signals:
    void DismissMsgButtonClicked();

private slots:
    // Handler to process the dismiss message button click.
    void HandleDismissMessagePushButtonClicked(/* bool checked */);

private:
    // Connect the signals.
    void ConnectSignals();

    // Set the mouse cursor to pointing hand cursor.
    void SetCursor();

    // The generated view object for the title bar.
    Ui::rgSourceEditorTitlebar ui;
};
