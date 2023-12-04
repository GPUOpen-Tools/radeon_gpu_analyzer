#pragma once

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"

// A class responsible for displaying ISA code for multiple GPU architectures.
class RgIsaDisassemblyViewBinary : public RgIsaDisassemblyView
{
    Q_OBJECT

public:
    RgIsaDisassemblyViewBinary(QWidget* parent);
    virtual ~RgIsaDisassemblyViewBinary() = default;

    // Populate the disassembly view using the given clone and build outputs.
    virtual bool PopulateBuildOutput(const std::shared_ptr<RgProjectClone> project_clone, const RgBuildOutputsMap& build_outputs) override;

protected:
    // Set the border stylesheet.
    virtual void SetBorderStylesheet(bool is_selected) override;

    // Populate the disassembly view with the given CLI build output.
    bool PopulateDisassemblyView(const std::string& binary_file_name, const RgBuildOutputsMap& build_output);
};
