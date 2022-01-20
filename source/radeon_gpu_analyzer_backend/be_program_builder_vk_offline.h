//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_VK_OFFLINE_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_VK_OFFLINE_H_

// C++.
#include <string>
#include <set>

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder.h"
#include "radeon_gpu_analyzer_backend/be_include.h"
#include "radeon_gpu_analyzer_backend/be_data_types.h"

struct VkOfflineOptions : public beKA::CompileOptions
{
    VkOfflineOptions(beKA::RgaMode mode)
    {
        CompileOptions::mode = mode;
    }

    // The target devices.
    std::string target_device_name;

    // The generic input file name.
    std::string stageless_input_file;

    // The input shader file names.
    BeProgramPipeline pipeline_shaders;

    // AMD PAL IL binary output file names.
    BeProgramPipeline pal_il_binaries_output_files;

    // AMD PAL IL disassembly output files.
    BeProgramPipeline pal_il_disassembly_output_files;

    // ISA disassembly output file names.
    BeProgramPipeline isa_disassembly_output_files;

    // Register liveness analysis output file names.
    BeProgramPipeline livereg_output_files;

    // Stall analysis output file names.
    BeProgramPipeline stall_output_files;

    // Control flow graph output file names.
    BeProgramPipeline cfg_output_files;

    // ISA binary output file name.
    BeProgramPipeline isa_binary_output_files;

    // SC statistics output file name.
    BeProgramPipeline stats_output_files;

    // Full path to input .pipe file (if given).
    std::string pipe_file;

    // Full path to output pipeline ELF binary file.
    std::string pipeline_binary;

    // True to generate SPIR-V binaries.
    bool is_spirv_binaries_required = false;

    // True to generate AMD PAL IL binaries.
    bool is_amd_pal_il_binaries_required = false;

    // True to generate AMD PAL IL disassembly.
    bool is_amd_pal_disassembly_required = false;

    // True to generate AMD ISA binaries.
    bool is_amd_isa_binaries_required = false;

    // True to generate the pipeline ELF binary.
    bool is_pipeline_binary_required = false;

    // True to generate AMD ISA binaries.
    bool is_amd_isa_disassembly_required = false;

    // True to perform live register analysis.
    bool is_livereg_required = false;

    // True to generate shader compiler statistics.
    bool is_stats_required = false;
};

class BeProgramBuilderVkOffline : public BeProgramBuilder
{
public:
    BeProgramBuilderVkOffline() = default;
    ~BeProgramBuilderVkOffline() = default;

    virtual beKA::beStatus GetKernelIlText(const std::string& device, const std::string& kernel, std::string& il) override;

    virtual beKA::beStatus GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa) override;

    virtual beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) override;

    virtual beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;

    beKA::beStatus Compile(const VkOfflineOptions& vulkan_options, bool& cancel_signal, bool should_print_cmd, gtString& build_log);

    /// Extracts the OpenGL version of the installed runtime.
    bool GetVulkanVersion(gtString& vk_version) const;

    /// Retrieves the list of supported devices.
    static bool GetSupportedDevices(std::set<std::string>& device_list);
};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_VK_OFFLINE_H_
