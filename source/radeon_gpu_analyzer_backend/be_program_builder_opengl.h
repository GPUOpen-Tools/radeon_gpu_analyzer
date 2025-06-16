//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for rga backend progam builder opengl class.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_OPENGL_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_OPENGL_H_

// C++.
#include <string>
#include <set>

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder.h"
#include "radeon_gpu_analyzer_backend/be_data_types.h"
#include "radeon_gpu_analyzer_backend/be_include.h"

struct OpenglOptions : public beKA::CompileOptions
{
    OpenglOptions() : chip_family(0), chip_revision(0), is_amd_isa_binaries_required(false),
        is_amd_isa_disassembly_required(false), is_il_disassembly_required(false), is_stats_required(false),
        is_cfg_required(false), is_livereg_required(false), is_livereg_sgpr_required(false), is_stalls_required(false)
    {
        CompileOptions::mode = beKA::RgaMode::kModeOpengl;
    }

    // The target device's chip family.
    size_t chip_family = 0;

    // The target device's chip revision.
    size_t chip_revision = 0;

    // The target device's name.
    std::string device_name;

    // The input shader file names.
    BeProgramPipeline pipeline_shaders;

    // ISA disassembly output file names.
    BeProgramPipeline isa_disassembly_output_files;

    // AMDIL disassembly output file names.
    BeProgramPipeline il_disassembly_output_files;

    // Register liveness analysis (vgpr) output file names.
    BeProgramPipeline livereg_output_files;

    // Register liveness analysis (sgpr) output file names.
    BeProgramPipeline livereg_sgpr_output_files;

    // Stall analysis output file names.
    BeProgramPipeline stall_analysis_output_files;

    // Register control flow graph output file names.
    BeProgramPipeline cfg_output_files;

    // SC statistics output file name.
    BeProgramPipeline stats_output_files;

    // ISA binary output file name.
    std::string program_binary_filename;

    // True to generate AMD ISA binaries.
    bool is_amd_isa_binaries_required = false;

    // True to generate AMD ISA disassembly.
    bool is_amd_isa_disassembly_required = false;

    // True to generate AMD IL disassembly.
    bool is_il_disassembly_required = false;

    // True to perform live register analysis.
    bool is_livereg_required = false;

    // True to perform live register analysis.
    bool is_livereg_sgpr_required = false;

    // True to perform stall analysis.
    bool is_stalls_required = false;

    // True to perform control flow graph.
    bool is_cfg_required = false;

    // True to generate shader compiler statistics.
    bool is_stats_required = false;

    // True to generate the pipeline ELF binary.
    bool is_pipeline_binary_required = false;
};

class BeProgramBuilderOpengl : public BeProgramBuilder
{
public:
    BeProgramBuilderOpengl();
    virtual ~BeProgramBuilderOpengl();

    virtual beKA::beStatus GetKernelIlText(const std::string& device, const std::string& kernel, std::string& il) override;

    virtual beKA::beStatus GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa) override;

    virtual beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) override;

    virtual beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;

    beKA::beStatus Compile(const OpenglOptions& vulkanOptions, bool& cancelSignal, bool printCmd, gtString& compilerOutput, gtString& build_log);

    /// Extracts the OpenGL version of the installed runtime.
    bool GetOpenGLVersion(bool printCmd, gtString& glVersion) const;

    // Retrieve the device name as recognized by the OpenGl backend.
    bool GetDeviceGLName(const std::string& device_name, std::string& valid_device_name) const;

    /// Retrieves the list of supported devices.
    static bool GetSupportedDevices(std::set<std::string>& deviceList);

    /// Retrieves the list of disabled devices.
    static const std::set<std::string>& GetDisabledDevices();
};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_OPENGL_H_
