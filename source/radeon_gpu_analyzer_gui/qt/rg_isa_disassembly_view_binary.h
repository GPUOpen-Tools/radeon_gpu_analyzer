//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for shader ISA Disassembly view for Binary Analysis.
//=============================================================================

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

    // Set the target gpu label and architecture in the model.
    void SetTargetGpuLabel(const std::string& target_gpu);

protected:
    // Set the border stylesheet.
    virtual void SetBorderStylesheet(bool is_selected) override;

    // Populate the disassembly view with the given CLI build output.
    bool PopulateDisassemblyView(const std::vector<RgBinaryFileInfo>& binary_files, const RgBuildOutputsMap& build_output);
};
