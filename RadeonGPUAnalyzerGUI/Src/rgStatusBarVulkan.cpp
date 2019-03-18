// C++.
#include <cassert>

// Qt.
#include <QStatusBar>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStatusBar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStatusBarVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgModePushButton.h>

static const QString s_MODE_PUSH_BUTTON_STYLESHEET(
    "QPushButton"
    "{"
    "padding: 5px;"
    "border: none;"
    "color: white;"
    "}"
    "QPushButton:hover"
    "{"
    "background-color: rgb(255, 36, 65, 255);"
    "}"
);
static const QString s_STATUS_BAR_BACKRGOUND_COLOR_VULKAN = "background-color: rgb(224,30,55)";
static const QColor s_MODE_PUSH_BUTTON_HOVER_COLOR = QColor(255, 36, 65, 255);
static const QColor s_MODE_PUSH_BUTTON_NON_HOVER_COLOR = QColor(224, 30, 55, 255);

rgStatusBarVulkan::rgStatusBarVulkan(QStatusBar* pStatusBar, QWidget* pParent) :
    rgStatusBar(pStatusBar, pParent),
    m_pParent(pParent)
{
    // Set style sheets.
    SetStylesheets(pStatusBar);
}

void rgStatusBarVulkan::SetStylesheets(QStatusBar* pStatusBar)
{
    assert(m_pModePushButton);
    if (m_pModePushButton)
    {
        m_pModePushButton->setStyleSheet(s_MODE_PUSH_BUTTON_STYLESHEET);
        m_pModePushButton->SetHoverColor(s_MODE_PUSH_BUTTON_HOVER_COLOR);
        m_pModePushButton->SetNonHoverColor(s_MODE_PUSH_BUTTON_NON_HOVER_COLOR);
    }

    // Set status bar stylesheet.
    assert(pStatusBar != nullptr);
    if (pStatusBar != nullptr)
    {
        pStatusBar->setStyleSheet(s_STATUS_BAR_BACKRGOUND_COLOR_VULKAN);
    }
}