// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QPushButton>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuBuildSettingsItem.h>

// Stylesheet for Build settings button when in focus.
static const char* s_BUTTON_FOCUS_STYLESHEET = "QPushButton { border: 1px solid #6666FF; margin: 1px; background: lightGray;}";

rgFileMenuBuildSettingsItem::rgFileMenuBuildSettingsItem(rgFileMenu* pParent) :
    rgFileMenuItem(pParent)
{
    ui.setupUi(this);

    // Set the status bar tip.
    this->setStatusTip(STR_MENU_BAR_BUILD_SETTINGS_TOOLTIP);

    // Set the tool tip.
    this->setToolTip(STR_MENU_BAR_BUILD_SETTINGS_TOOLTIP);

    // Set the mouse cursor to pointing hand cursor.
    SetCursor();

    // Connect the file menu signals.
    ConnectSignals();
}

void rgFileMenuBuildSettingsItem::ConnectSignals()
{
    // Connect the "Build settings" push button clicked signal.
    bool isConnected = connect(ui.buildSettingsButton, &QPushButton::clicked, this, &rgFileMenuBuildSettingsItem::HandleBuildSettingsButton);
    assert(isConnected);
}

void rgFileMenuBuildSettingsItem::HandleBuildSettingsButton(bool checked)
{
    Q_UNUSED(checked);

    ui.buildSettingsButton->setStyleSheet(s_BUTTON_FOCUS_STYLESHEET);
}

QPushButton* rgFileMenuBuildSettingsItem::GetBuildSettingsButton() const
{
    return ui.buildSettingsButton;
}

void rgFileMenuBuildSettingsItem::SetHasPendingChanges(bool hasPendingChanges)
{
    std::stringstream itemText;

    itemText << STR_MENU_BUILD_SETTINGS;

    if (hasPendingChanges)
    {
        itemText << STR_UNSAVED_FILE_SUFFIX;
    }

    SetItemText(itemText.str().c_str());
}

void rgFileMenuBuildSettingsItem::SetItemText(const std::string& itemText)
{
    ui.buildSettingsButton->setText(itemText.c_str());
}

void rgFileMenuBuildSettingsItem::SetCursor()
{
    // Set the mouse cursor to pointing hand cursor.
    ui.buildSettingsButton->setCursor(Qt::PointingHandCursor);
}