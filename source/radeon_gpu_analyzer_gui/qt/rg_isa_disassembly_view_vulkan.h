#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_VULKAN_H_

// Qt.
#include <QWidget>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_graphics.h"

// A class responsible for displaying ISA code for multiple GPU architectures.
class RgIsaDisassemblyViewVulkan : public RgIsaDisassemblyViewGraphics
{
    Q_OBJECT

public:
    RgIsaDisassemblyViewVulkan(QWidget* parent);
    virtual ~RgIsaDisassemblyViewVulkan() = default;

protected:
    // Set the border stylesheet.
    virtual void SetBorderStylesheet(bool is_selected) override;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_VULKAN_H_
