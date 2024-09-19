// C++.
#include <cassert>

// Qt.
#include <QStatusBar>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_mode_push_button.h"

static const QString kStrModePushButtonStylesheet(
    "QPushButton"
    "{"
    "padding: 5px;"
    "border: none;"
    "color: white;"
    "}"
    "QPushButton:hover"
    "{"
    "background-color: rgba(255, 36, 65, 255);"
    "}"
);
static const QString kStrStatusBarBackgroundColorVulkan = "background-color: rgb(224,30,55); color: white;";
static const QColor kModePushButtonHoverColor = QColor(255, 36, 65, 255);
static const QColor kModePushButtonNonHoverColor = QColor(224, 30, 55, 255);

RgStatusBarVulkan::RgStatusBarVulkan(QStatusBar* status_bar, QWidget* parent) :
    RgStatusBar(status_bar, parent),
    parent_(parent)
{
    // Set style sheets.
    SetStylesheets(status_bar);
}

void RgStatusBarVulkan::SetStylesheets(QStatusBar* status_bar)
{
    assert(mode_push_button_);
    if (mode_push_button_)
    {
        mode_push_button_->setStyleSheet(kStrModePushButtonStylesheet);
        mode_push_button_->SetHoverColor(kModePushButtonHoverColor);
        mode_push_button_->SetNonHoverColor(kModePushButtonNonHoverColor);
    }

    // Set status bar stylesheet.
    assert(status_bar != nullptr);
    if (status_bar != nullptr)
    {
        status_bar->setStyleSheet(kStrStatusBarBackgroundColorVulkan);
    }
}
