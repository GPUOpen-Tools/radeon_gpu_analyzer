#include "rgAddCreateMenuItem.h"

#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

rgAddCreateMenuItem::rgAddCreateMenuItem(rgMenu* pParent) :
    rgMenuItem(pParent)
{
    ui.setupUi(this);
    ui.addButton->setContentsMargins(0, 0, 0, 0);
    ui.createButton->setContentsMargins(0, 0, 0, 0);

    // Set button tooltips and status tips.
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_OPEN_EXISTING_FILE_TOOLTIP_OPENCL, ui.addButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_CREATE_NEW_FILE_TOOLTIP_OPENCL, ui.createButton);

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