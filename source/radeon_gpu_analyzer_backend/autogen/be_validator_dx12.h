//=============================================================================
/// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for dx12 pipeline auto generator class.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_VALIDATOR_DX12_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_VALIDATOR_DX12_H_

// CLI.
#include "source/radeon_gpu_analyzer_cli/kc_config.h"

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_include_dx12.h"

class BeDx12PipelineValidator
{
public:
    // Allows RGA to check and validate inputs for a Dx12 pipeline.
    // This function return true if all inputs are valid.
    // This functions also tries autogenerating other required files to complete the pipeline.
    static bool ValidateAndGenerateDx12Pipeline(const Config& config, bool is_dxr_pipeline, BeDx12AutoGenPipelineInfo& info);

private:
    // Allows RGA to compile a single DX12 shader by autogenerating other required files.
    static void AutoGenerateMissingPipeline(const Config& config, BeDx12AutoGenPipelineInfo& info);
};

#endif  // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_VALIDATOR_DX12_H_
