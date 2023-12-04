#include "radeon_gpu_analyzer_gui/qt/rg_link_source_menu_item.h"

#include "radeon_gpu_analyzer_gui/rg_data_types_binary.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgLinkSourceMenuItem::RgLinkSourceMenuItem(RgMenu* parent)
    :RgMenuItem(parent)
{
    ui_.setupUi(this);
    ui_.loadCodeObjectButton->setContentsMargins(0, 0, 0, 0);
    ui_.linkSourceButton->setContentsMargins(0, 0, 0, 0);

    // Set button tooltips and status tips.
    RgUtils::SetToolAndStatusTip(kStrMenuBarOpenExistingFileTooltipBinary, ui_.loadCodeObjectButton);
    RgUtils::SetToolAndStatusTip(kStrMenuBarLinkSourceFileTooltipBinary, ui_.linkSourceButton);

    // Set mouse pointer to pointing hand cursor.
    SetCursor();
}

QPushButton* RgLinkSourceMenuItem::GetLoadCodeObjButton() const
{
    return ui_.loadCodeObjectButton;
}

QPushButton* RgLinkSourceMenuItem::GetLinkSourceButton() const
{
    return ui_.linkSourceButton;
}

void RgLinkSourceMenuItem::ToggleLineSeparatorVisibilty(bool visible) const
{
    ui_.line->setVisible(visible);
    ui_.line_2->setVisible(visible);
}

void RgLinkSourceMenuItem::ToggleLoadCodeObjectButtonVisibilty(bool visible) const
{
    ui_.loadCodeObjectButton->setVisible(visible);
}

void RgLinkSourceMenuItem::SetCursor()
{
    // Set mouse pointer to pointing hand cursor.
    ui_.loadCodeObjectButton->setCursor(Qt::PointingHandCursor);
    ui_.linkSourceButton->setCursor(Qt::PointingHandCursor);
}
