// C++.
#include <cassert>
// Qt.
#include <QStatusBar>
#include <QColor>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_mode_push_button.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

static const QString kStrModePushButtonStylesheet(
    "QPushButton"
    "{"
    "padding: 5px;"
    "border: none;"
    "color: white;"
    "}"
    "QPushButton:hover"
    "{"
    "background-color: rgba(160, 0, 160, 255);"
    "}"
);
static const QString kStrStatusBarBackgroundColorBinary = "background-color: rgb(128, 0, 128)";
static const QColor kModePushButtonHoverColor = QColor(160, 0, 160, 255);
static const QColor kModePushButtonNonHoverColor = QColor(128, 0, 128);

RgStatusBarBinary::RgStatusBarBinary(QStatusBar* status_bar, QWidget* parent)
    :
    RgStatusBar(status_bar, parent),
    parent_(parent)
{
    // Set style sheets.
    SetStylesheets(status_bar);
}

void RgStatusBarBinary::SetStylesheets(QStatusBar* status_bar)
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
        status_bar->setStyleSheet(kStrStatusBarBackgroundColorBinary);
    }
}

bool RgStatusBarBinary::ConstructStatusMessageString(StatusType type, std::string& status_msg_str) const
{
    bool ret = true;
    switch (type)
    {
    case StatusType::kStarted:
        status_msg_str = std::string{kStrStatusBarAnalysis} + std::string{kStrStatusBarStarted};
        break;
    case StatusType::kFailed:
        status_msg_str = std::string{kStrStatusBarAnalysis} + std::string{kStrStatusBarFailed};
        break;
    case StatusType::kCanceled:
        status_msg_str = std::string{kStrStatusBarAnalysis} + std::string{kStrStatusBarCanceled};
        break;
    case StatusType::kSucceeded:
        status_msg_str = std::string{kStrStatusBarAnalysis} + std::string{kStrStatusBarSucceeded};
        break;
    case StatusType::kUnknown:
    default:
        // We shouldn't get here.
        ret = false;
        assert(ret);
        break;
    }
    return ret;
}