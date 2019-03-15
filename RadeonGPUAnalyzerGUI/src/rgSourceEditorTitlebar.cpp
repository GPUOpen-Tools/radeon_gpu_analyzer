// C++.
#include <cassert>
#include <sstream>

// Local
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceEditorTitlebar.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

rgSourceEditorTitlebar::rgSourceEditorTitlebar(QWidget* pParent) :
    QFrame(pParent)
{
    ui.setupUi(this);

    // Initialize the title bar text and tooltip.
    std::stringstream titlebarText;
    titlebarText << STR_SOURCE_EDITOR_TITLEBAR_CORRELATION_DISABLED_A;
    titlebarText << STR_SOURCE_EDITOR_TITLEBAR_CORRELATION_DISABLED_B;
    ui.sourceCorrelationLabel->setText(titlebarText.str().c_str());

    // Initialize the title bar contents to be hidden.
    SetTitlebarContentsVisibility(false);

    // Set the mouse cursor to pointing hand cursor.
    SetCursor();

    // Connect the signals.
    ConnectSignals();

    // Prep the dismiss message push button.
    ui.dismissMessagePushButton->setIcon(QIcon(":/icons/deleteIcon.svg"));
    ui.dismissMessagePushButton->setStyleSheet("border: none");
}

void rgSourceEditorTitlebar::ConnectSignals()
{
    bool isConnected = connect(ui.dismissMessagePushButton, &QPushButton::clicked, this, &rgSourceEditorTitlebar::HandleDismissMessagePushButtonClicked);
    assert(isConnected);

    isConnected = connect(ui.dismissMessagePushButton, &QPushButton::clicked, this, &rgSourceEditorTitlebar::DismissMsgButtonClicked);
    assert(isConnected);
}

void rgSourceEditorTitlebar::HandleDismissMessagePushButtonClicked(/* bool checked */)
{
    SetTitlebarContentsVisibility(false);
}

void rgSourceEditorTitlebar::SetIsCorrelationEnabled(bool isEnabled)
{
    // Only show the title bar content if source line correlation is disabled.
    SetTitlebarContentsVisibility(!isEnabled);
}

void rgSourceEditorTitlebar::ShowMessage(const std::string& msg)
{
    ui.sourceCorrelationLabel->setText(msg.c_str());
    SetTitlebarContentsVisibility(true);
}

void rgSourceEditorTitlebar::SetCursor()
{
    // Set the mouse cursor to pointing hand cursor.
    ui.viewMaximizeButton->setCursor(Qt::PointingHandCursor);
    ui.dismissMessagePushButton->setCursor(Qt::PointingHandCursor);
}

void rgSourceEditorTitlebar::SetTitlebarContentsVisibility(bool isVisible)
{
    ui.sourceCorrelationIcon->setVisible(isVisible);
    ui.sourceCorrelationLabel->setVisible(isVisible);
    ui.dismissMessagePushButton->setVisible(isVisible);
}