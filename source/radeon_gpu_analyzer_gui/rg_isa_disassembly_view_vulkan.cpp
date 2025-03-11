// Qt.
#include <QStyle>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

RgIsaDisassemblyViewVulkan::RgIsaDisassemblyViewVulkan(QWidget* parent)
    : RgIsaDisassemblyViewGraphics(parent)
{
}

void RgIsaDisassemblyViewVulkan::SetBorderStylesheet(bool is_selected)
{
    // Set "selected" property to be utilized by this widget's stylesheet.
    ui_.frame->setProperty(kStrDisassemblyFrameSelected, is_selected);

    // Repolish the widget to ensure the style gets updated.
    ui_.frame->style()->unpolish(ui_.frame);
    ui_.frame->style()->polish(ui_.frame);
}
