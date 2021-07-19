#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_OPENCL_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"

// A class responsible for displaying ISA code for multiple GPU architectures.
class RgIsaDisassemblyViewOpencl : public RgIsaDisassemblyView
{
    Q_OBJECT

public:
    RgIsaDisassemblyViewOpencl(QWidget* parent);
    virtual ~RgIsaDisassemblyViewOpencl() = default;

    // Populate the disassembly view using the given clone and build outputs.
    virtual bool PopulateBuildOutput(const std::shared_ptr<RgProjectClone> project_clone, const RgBuildOutputsMap& build_outputs) override;

protected:
    // Set the border stylesheet.
    virtual void SetBorderStylesheet(bool is_selected) override;

    // Populate the disassembly view with the given CLI build output.
    bool PopulateDisassemblyView(const std::vector<RgSourceFileInfo>& source_files, const RgBuildOutputsMap& build_output);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_OPENCL_H_
