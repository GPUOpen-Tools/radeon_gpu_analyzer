#include "radeon_gpu_analyzer_gui/qt/rg_add_create_menu_item.h"

// QtCommon.
#include "qt_common/utils/qt_util.h"

#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgAddCreateMenuItem::RgAddCreateMenuItem(RgMenu* parent)
    : RgMenuItem(parent)
{
    ui_.setupUi(this);
    ui_.addButton->setContentsMargins(0, 0, 0, 0);
    ui_.createButton->setContentsMargins(0, 0, 0, 0);

    ColorThemeType color_theme = QtCommon::QtUtils::ColorTheme::Get().GetColorTheme();

    if (color_theme == kColorThemeTypeDark)
    {
        ui_.addButton->setIcon(QIcon(":/icons/add_file_icon_dark_mode.svg"));
        ui_.createButton->setIcon(QIcon(":/icons/new_file_icon_dark_mode.svg"));
    }

    // Set button tooltips and status tips.
    RgUtils::SetToolAndStatusTip(kStrMenuBarOpenExistingFileTooltipOpencl, ui_.addButton);
    RgUtils::SetToolAndStatusTip(kStrMenuBarCreateNewFileTooltipOpencl, ui_.createButton);

    // Set mouse pointer to pointing hand cursor.
    SetCursor();
}

QPushButton* RgAddCreateMenuItem::GetAddButton() const
{
    return ui_.addButton;
}

QPushButton* RgAddCreateMenuItem::GetCreateButton() const
{
    return ui_.createButton;
}

void RgAddCreateMenuItem::SetCursor()
{
    // Set mouse pointer to pointing hand cursor.
    ui_.addButton->setCursor(Qt::PointingHandCursor);
    ui_.createButton->setCursor(Qt::PointingHandCursor);
}
