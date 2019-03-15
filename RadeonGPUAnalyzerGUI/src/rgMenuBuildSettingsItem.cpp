// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QPushButton>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuBuildSettingsItem.h>

static const char* s_BUTTON_FOCUS_IN_STYLESHEET_GRAPHICS = "QPushButton { background: rgb(253,255,174); border-style: solid; border-width: 2px; border-color: rgb(135, 20, 16);}";
static const char* s_BUTTON_FOCUS_OUT_STYLESHEET = "QPushButton { margin: 1px; background: rgb(214, 214, 214);}";
static const char* s_BUILD_SETTINGS_BUTTON_NAME = "buildSettingsButton";

rgMenuBuildSettingsItem::rgMenuBuildSettingsItem(rgMenu* pParent) :
    rgMenuItem(pParent)
{
    ui.setupUi(this);

    // Set the status bar tip.
    this->setStatusTip(STR_MENU_BAR_BUILD_SETTINGS_TOOLTIP);

    // Set the tool tip.
    this->setToolTip(STR_MENU_BAR_BUILD_SETTINGS_TOOLTIP);

    // Set the mouse cursor to pointing hand cursor.
    SetCursor(Qt::PointingHandCursor);

    // Connect the file menu signals.
    ConnectSignals();

    // Set object name.
    setObjectName(s_BUILD_SETTINGS_BUTTON_NAME);
}

void rgMenuBuildSettingsItem::ConnectSignals()
{
    // Connect the "Build settings" push button clicked signal.
    bool isConnected = connect(ui.buildSettingsButton, &QPushButton::clicked, this, &rgMenuBuildSettingsItem::HandleBuildSettingsButton);
    assert(isConnected);

    // Connect the "Build settings" button to forward the click externally for the file menu to handle.
    isConnected = connect(ui.buildSettingsButton, &QPushButton::clicked, this, &rgMenuBuildSettingsItem::BuildSettingsButtonClicked);
    assert(isConnected);
}

void rgMenuBuildSettingsItem::HandleBuildSettingsButton(bool checked)
{
    Q_UNUSED(checked);

    ui.buildSettingsButton->setCursor(Qt::ArrowCursor);

    SetCurrent(true);
}

QPushButton* rgMenuBuildSettingsItem::GetBuildSettingsButton() const
{
    return ui.buildSettingsButton;
}

void rgMenuBuildSettingsItem::SetHasPendingChanges(bool hasPendingChanges)
{
    std::stringstream itemText;

    itemText << STR_MENU_BUILD_SETTINGS;

    if (hasPendingChanges)
    {
        itemText << STR_UNSAVED_FILE_SUFFIX;
    }

    SetItemText(itemText.str().c_str());
}

void rgMenuBuildSettingsItem::SetItemText(const std::string& itemText)
{
    ui.buildSettingsButton->setText(itemText.c_str());
}

void rgMenuBuildSettingsItem::GotFocus()
{
    // Set arrow cursor so it doesn't appear that the user can click on the button again.
    ui.buildSettingsButton->setCursor(Qt::ArrowCursor);

    // Set stylesheet.
    ui.buildSettingsButton->setStyleSheet(s_BUTTON_FOCUS_IN_STYLESHEET_GRAPHICS);
}

void rgMenuBuildSettingsItem::LostFocus()
{
    // Set pointing hand cursor so it looks like the user can click on it.
    ui.buildSettingsButton->setCursor(Qt::PointingHandCursor);

    // Set stylesheet.
    ui.buildSettingsButton->setStyleSheet(s_BUTTON_FOCUS_OUT_STYLESHEET);
}

void rgMenuBuildSettingsItem::SetCursor(const QCursor& cursor)
{
    // Set the mouse cursor to the specified type.
    ui.buildSettingsButton->setCursor(cursor);
}

void rgMenuBuildSettingsItem::SetCurrent(bool isCurrent)
{
    m_current = isCurrent;

    if (m_current)
    {
        GotFocus();
    }
    else
    {
        LostFocus();
    }
}

bool rgMenuBuildSettingsItem::IsCurrent() const
{
    return m_current;
}

void rgMenuBuildSettingsItem::ClickMenuItem() const
{
    QPushButton* pBuildSettingsButton = GetBuildSettingsButton();
    assert(pBuildSettingsButton != nullptr);
    if (pBuildSettingsButton != nullptr)
    {
        pBuildSettingsButton->click();
    }
}
