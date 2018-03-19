// C++.
#include <sstream>

// Local
#include <RadeonGPUAnalyzerGUI/include/qt/rgSourceEditorTitlebar.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

rgSourceEditorTitlebar::rgSourceEditorTitlebar(QWidget* pParent) :
    QFrame(pParent)
{
    ui.setupUi(this);

    // Initialize the title bar text and tooltip.
    std::stringstream titlebarText;
    titlebarText << STR_SOURCE_EDITOR_TITLEBAR_CORRELATION_DISABLED_A;
    titlebarText << STR_SOURCE_EDITOR_TITLEBAR_CORRELATION_DISABLED_B;
    ui.sourceCorrelationLabel->setText(titlebarText.str().c_str());

    rgUtils::SetToolAndStatusTip(titlebarText.str().c_str(), ui.correlationGroupWidget);

    // Initialize the title bar contents to be hidden.
    SetTitlebarContentsVisibility(false);

    // Set the mouse cursor to pointing hand cursor.
    SetCursor();
}

void rgSourceEditorTitlebar::SetIsCorrelationEnabled(bool isEnabled)
{
    // Only show the title bar content if source line correlation is disabled.
    SetTitlebarContentsVisibility(!isEnabled);
}

void rgSourceEditorTitlebar::SetCursor()
{
    // Set the mouse cursor to pointing hand cursor.
    ui.viewMaximizeButton->setCursor(Qt::PointingHandCursor);
}

void rgSourceEditorTitlebar::SetTitlebarContentsVisibility(bool isVisible)
{
    ui.sourceCorrelationIcon->setVisible(isVisible);
    ui.sourceCorrelationLabel->setVisible(isVisible);
}