#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_GRAPHICS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_GRAPHICS_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"

// A class responsible for displaying ISA code for graphics or compute pipelines.
class RgIsaDisassemblyViewGraphics : public RgIsaDisassemblyView
{
    Q_OBJECT

public:
    explicit RgIsaDisassemblyViewGraphics(QWidget* parent = nullptr);
    virtual ~RgIsaDisassemblyViewGraphics() = default;

    // Populate the disassembly view using the given clone and build outputs.
    virtual bool PopulateBuildOutput(const std::shared_ptr<RgProjectClone> project_clone, const RgBuildOutputsMap& build_outputs) override;

private:
    bool PopulateDisassemblyView(const ShaderInputFileArray& shader_stage_array, const RgBuildOutputsMap& build_output);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_GRAPHICS_H_
