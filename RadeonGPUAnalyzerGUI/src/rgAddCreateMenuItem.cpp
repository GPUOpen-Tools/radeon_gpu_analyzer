#include "rgAddCreateMenuItem.h"

#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

rgAddCreateMenuItem::rgAddCreateMenuItem(rgFileMenu* pParent) :
    rgFileMenuItem(pParent)
{
    ui.setupUi(this);
    ui.addButton->setContentsMargins(0, 0, 0, 0);
    ui.createButton->setContentsMargins(0, 0, 0, 0);

    // Set button tooltips and status tips.
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_OPEN_EXISTING_FILE_TOOLTIP, ui.addButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_CREATE_NEW_FILE_TOOLTIP, ui.createButton);

    // Set mouse pointer to pointing hand cursor.
    SetCursor();
}

QPushButton* rgAddCreateMenuItem::GetAddButton() const
{
    return ui.addButton;
}

QPushButton* rgAddCreateMenuItem::GetCreateButton() const
{
    return ui.createButton;
}

void rgAddCreateMenuItem::SetCursor()
{
    // Set mouse pointer to pointing hand cursor.
    ui.addButton->setCursor(Qt::PointingHandCursor);
    ui.createButton->setCursor(Qt::PointingHandCursor);
}