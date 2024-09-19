// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QPushButton>

// QtCommon.
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_build_settings_item.h"

static const char* kStrButtonFocusInStylesheetGraphics =
    "QPushButton { background: palette(highlight); border-style: solid; border-width: 2px; border-color: rgb(135, 20, 16);}";
static const char* kStrButtonFocusOutStylesheet = "QPushButton {background: palette(button); margin: 1px; }";
static const char* kStrBuildSettingsButtonName  = "buildSettingsButton";

RgMenuBuildSettingsItem::RgMenuBuildSettingsItem(RgMenu* parent, QString tooltip)
    : RgMenuItem(parent)
{
    ui_.setupUi(this);

    // Set the status bar tip.
    this->setStatusTip(tooltip);

    // Set the tool tip.
    this->setToolTip(tooltip);

    ColorThemeType color_theme = QtCommon::QtUtils::ColorTheme::Get().GetColorTheme();

    if (color_theme == kColorThemeTypeDark)
    {
        ui_.buildSettingsButton->setIcon(QIcon(":/icons/gear_icon_white.svg"));
    }

    // Set the mouse cursor to pointing hand cursor.
    SetCursor(Qt::PointingHandCursor);

    // Connect the file menu signals.
    ConnectSignals();

    // Set object name.
    setObjectName(kStrBuildSettingsButtonName);
}

void RgMenuBuildSettingsItem::ConnectSignals()
{
    // Connect the "Build settings" push button clicked signal.
    bool is_connected = connect(ui_.buildSettingsButton, &QPushButton::clicked, this, &RgMenuBuildSettingsItem::HandleBuildSettingsButtonClicked);
    assert(is_connected);

    // Connect the "Build settings" button to forward the click externally for the file menu to handle.
    is_connected = connect(ui_.buildSettingsButton, &QPushButton::clicked, this, &RgMenuBuildSettingsItem::BuildSettingsButtonClicked);
    assert(is_connected);
}

void RgMenuBuildSettingsItem::HandleBuildSettingsButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    ui_.buildSettingsButton->setCursor(Qt::ArrowCursor);

    SetCurrent(true);

    // Enable the pipeline state menu item in Build menu.
    emit EnablePipelineMenuItem(true);

    // Disable the build settings menu item in Build menu.
    emit EnableBuildSettingsMenuItem(false);
}

QPushButton* RgMenuBuildSettingsItem::GetBuildSettingsButton() const
{
    return ui_.buildSettingsButton;
}

void RgMenuBuildSettingsItem::SetHasPendingChanges(bool has_pending_changes)
{
    std::stringstream item_text;

    item_text << kStrMenuBuildSettings;

    if (has_pending_changes)
    {
        item_text << kStrUnsavedFileSuffix;
    }

    SetItemText(item_text.str().c_str());
}

void RgMenuBuildSettingsItem::SetItemText(const std::string& item_text)
{
    ui_.buildSettingsButton->setText(item_text.c_str());
}

void RgMenuBuildSettingsItem::GotFocus()
{
    // Set arrow cursor so it doesn't appear that the user can click on the button again.
    ui_.buildSettingsButton->setCursor(Qt::ArrowCursor);

    // Set stylesheet.
    ui_.buildSettingsButton->setStyleSheet(kStrButtonFocusInStylesheetGraphics);
}

void RgMenuBuildSettingsItem::LostFocus()
{
    // Set pointing hand cursor so it looks like the user can click on it.
    ui_.buildSettingsButton->setCursor(Qt::PointingHandCursor);

    // Set stylesheet.
    ui_.buildSettingsButton->setStyleSheet(kStrButtonFocusOutStylesheet);
}

void RgMenuBuildSettingsItem::SetCursor(const QCursor& cursor)
{
    // Set the mouse cursor to the specified type.
    ui_.buildSettingsButton->setCursor(cursor);
}

void RgMenuBuildSettingsItem::SetCurrent(bool is_current)
{
    current_ = is_current;

    if (current_)
    {
        GotFocus();
    }
    else
    {
        LostFocus();
    }
}

bool RgMenuBuildSettingsItem::IsCurrent() const
{
    return current_;
}

void RgMenuBuildSettingsItem::ClickMenuItem() const
{
    QPushButton* build_settings_button = GetBuildSettingsButton();
    assert(build_settings_button != nullptr);
    if (build_settings_button != nullptr)
    {
        build_settings_button->click();
    }
}
