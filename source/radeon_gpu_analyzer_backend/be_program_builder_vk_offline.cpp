//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <sstream>
#include <cassert>

// Infra.
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4309)
#endif
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_os_wrappers/Include/osProcess.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder_vk_offline.h"
#include "radeon_gpu_analyzer_backend/be_include.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_string_constants.h"
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"

// Device info.
#include "DeviceInfoUtils.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

static const std::string  kStrVkOfflineSpirvOutputFilenameVertex = "vert.spv";
static const std::string  kStrVkOfflineSpirvOutputFilenameTessellationControl = "tesc.spv";
static const std::string  kStrVkOfflineSpirvOutputFilenameTessellationEvaluation = "tese.spv";
static const std::string  kStrVkOfflineSpirvOutputFilenameGeometry = "geom.spv";
static const std::string  kStrVkOfflineSpirvOutputFilenameFragment = "frag.spv";
static const std::string  kStrVkOfflineSpirvOutputFilenameCompute = "comp.spv";

static const std::string  kStrPalIlOutputFilenameVert = "vert.palIl";
static const std::string  kStrPalIlOutputFilenameTessellationControl = "tesc.palIl";
static const std::string  kStrPalIlOutputFilenameTessellationEvaluation = "tese.palIl";
static const std::string  kStrPalIlOutputFilenameGeometry = "geom.palIl";
static const std::string  kStrPalIlOutputFilenameFragment = "frag.palIl";
static const std::string  kStrPalIlOutputFilenameCompute = "comp.palIl";

static const std::string  kAmdspvDeviceGfx900 = "900";
static const std::string  kAmdspvDeviceGfx902 = "902";
static const std::string  kAmdspvDeviceGfx906 = "906";

static const std::string  kAmdspvDeviceGfx1010 = "1010";
static const std::string  kAmdspvDeviceGfx1011 = "1011";
static const std::string  kAmdspvDeviceGfx1012 = "1012";
static const std::string  kAmdspvDeviceGfx1030 = "1030";
static const std::string  kAmdspvDeviceGfx1031 = "1031";
static const std::string  kAmdspvDeviceGfx1032 = "1032";
static const std::string  kAmdspvDeviceGfx1034 = "1034";

static bool GetAmdspvPath(std::string& amdspv_path)
{
#ifdef __linux
    amdspv_path = "amdspv";
#elif _WIN64
    amdspv_path = "utils\\amdspv.exe";
#elif _WIN32
    amdspv_path = "x86\\amdspv.exe";
#endif
    return true;
}

static bool GetGfxIpForVulkan(AMDTDeviceInfoUtils* device_info, const VkOfflineOptions& vulkan_options, std::string& gfx_ip_str)
{
    bool ret = false;
    gfx_ip_str.clear();

    if (vulkan_options.target_device_name.compare(kDeviceNameKalindi) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameGodavari) == 0)
    {
        // Special case #1: 7.x devices.
        gfx_ip_str = "7.x";
        ret = true;
    }
    else if (vulkan_options.target_device_name.compare(kDeviceNameStoney) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameAmur) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameNolan) == 0)
    {
        // Special case #2: 8.1 devices.
        gfx_ip_str = "8";
        ret = true;
    }
    else if (vulkan_options.target_device_name.compare(kDeviceNameGfx900) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameGfx902) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameGfx906) == 0)
    {
        // Special case #3: gfx9 devices.
        gfx_ip_str =
            vulkan_options.target_device_name == kDeviceNameGfx900 ? kAmdspvDeviceGfx900 :
            vulkan_options.target_device_name == kDeviceNameGfx902 ? kAmdspvDeviceGfx902 :
            vulkan_options.target_device_name == kDeviceNameGfx906 ? kAmdspvDeviceGfx906 :
            "";
        ret = !gfx_ip_str.empty();
    }
    else if (vulkan_options.target_device_name.compare(kDeviceNameGfx1010) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameGfx1011) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameGfx1012) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameGfx1030) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameGfx1031) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameGfx1032) == 0 ||
        vulkan_options.target_device_name.compare(kDeviceNameGfx1034) == 0)
    {
        // Special case #4: gfx10 devices.
        gfx_ip_str =
            vulkan_options.target_device_name == kDeviceNameGfx1010 ? kAmdspvDeviceGfx1010 :
            vulkan_options.target_device_name == kDeviceNameGfx1011 ? kAmdspvDeviceGfx1011 :
            vulkan_options.target_device_name == kDeviceNameGfx1012 ? kAmdspvDeviceGfx1012 :
            vulkan_options.target_device_name == kDeviceNameGfx1030 ? kAmdspvDeviceGfx1030 :
            vulkan_options.target_device_name == kDeviceNameGfx1031 ? kAmdspvDeviceGfx1031 :
            vulkan_options.target_device_name == kDeviceNameGfx1032 ? kAmdspvDeviceGfx1032 :
            vulkan_options.target_device_name == kDeviceNameGfx1034 ? kAmdspvDeviceGfx1034 :
            "";
        ret = !gfx_ip_str.empty();
    }
    else
    {
        // The standard case.
        size_t device_gfx_ip = 0;
        GDT_HW_GENERATION hw_generation;
        bool is_device_hw_generation_extracted = device_info->GetHardwareGeneration(vulkan_options.target_device_name.c_str(), hw_generation) &&
            BeUtils::GdtHwGenToNumericValue(hw_generation, device_gfx_ip);

        if (is_device_hw_generation_extracted && device_gfx_ip > 0)
        {
            gfx_ip_str = std::to_string(device_gfx_ip);
            ret = true;
        }
    }

    return ret;
}

// An internal auxiliary function that returns the correct input prefix for the backend invocation,
// according to the input type. If the input type is GLSL, it simply returns the given GLSL prefix.
// Otherwise, it returns the relevant, fixed, prefix.
static std::string GetInputPrefix(const VkOfflineOptions& vulkan_options, const std::string& glsl_prefix)
{
    const char* kSpirvBinaryInputPrefix = "in.spv=\"";
    const char* kSpirvTextualInputPrefix = "in.spvText=\"";

    std::string ret;
    if (vulkan_options.mode == beKA::RgaMode::kModeVkOffline)
    {
        ret = glsl_prefix;
    }
    else if (vulkan_options.mode == beKA::RgaMode::kModeVkOfflineSpv)
    {
        ret = kSpirvBinaryInputPrefix;
    }
    else if (vulkan_options.mode == beKA::RgaMode::kModeVkOfflineSpvTxt)
    {
        ret = kSpirvTextualInputPrefix;
    }
    return ret;
}

static beKA::beStatus  AddInputFileNames(const VkOfflineOptions& options, std::stringstream& cmd)
{
    beKA::beStatus  status = beKA::kBeStatusSuccess;

    // If .pipe input, we don't need to add any other file.
    bool is_pipe_input = !options.pipe_file.empty();

    // Indicates that a stage-less input file name was provided.
    bool  is_non_stage_input = false;

    // Indicates that some of stage-specific file names was provided (--frag, --vert, etc.).
    bool is_stage_input = false;

    if (options.mode == beKA::kModeVkOfflineSpv ||
        options.mode == beKA::kModeVkOfflineSpvTxt)
    {
        if (!options.stageless_input_file.empty())
        {
            cmd << GetInputPrefix(options, "") << options.stageless_input_file << "\" ";
            is_non_stage_input = true;
        }
    }

    // You cannot mix compute and non-compute shaders in Vulkan,
    // so this has to be mutually exclusive.
    if (options.pipeline_shaders.compute_shader.isEmpty() || is_pipe_input)
    {
        // Vertex shader.
        if (!options.pipeline_shaders.vertex_shader.isEmpty())
        {
            cmd << GetInputPrefix(options, "in.vert.glsl=\"") << options.pipeline_shaders.vertex_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Tessellation control shader.
        if (!options.pipeline_shaders.tessellation_control_shader.isEmpty())
        {
            cmd << GetInputPrefix(options, "in.tesc.glsl=\"") << options.pipeline_shaders.tessellation_control_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Tessellation evaluation shader.
        if (!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty())
        {
            cmd << GetInputPrefix(options, "in.tese.glsl=\"") << options.pipeline_shaders.tessellation_evaluation_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Geometry shader.
        if (!options.pipeline_shaders.geometry_shader.isEmpty())
        {
            cmd << GetInputPrefix(options, "in.geom.glsl=\"") << options.pipeline_shaders.geometry_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Fragment shader.
        if (!options.pipeline_shaders.fragment_shader.isEmpty())
        {
            cmd << GetInputPrefix(options, "in.frag.glsl=\"") << options.pipeline_shaders.fragment_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }
    }
    else
    {
        // Compute shader.
        cmd << GetInputPrefix(options, "in.comp.glsl=\"") << options.pipeline_shaders.compute_shader.asASCIICharArray() << "\" ";
        is_stage_input = true;
    }

    if (!is_pipe_input)
    {
        if (!is_non_stage_input && !is_stage_input)
        {
            status = beKA::kBeStatusVulkanNoInputFile;
        }
        else if (is_non_stage_input && is_stage_input)
        {
            status = beKA::KBeStatusVulkanMixedInputFiles;
        }
    }

    return status;
}

static void AddOutputFileNames(const VkOfflineOptions& options, std::stringstream& cmd)
{
    bool is_spirv = (options.mode == beKA::kModeVkOfflineSpv ||
        options.mode == beKA::kModeVkOfflineSpvTxt);

    bool is_pipe_file = !is_spirv && !options.pipe_file.empty();

    auto add_output_file = [&](bool flag, const std::string& option, const std::string& fileName)
    {
        if (flag || is_spirv || is_pipe_file)
        {
            cmd << option << "\"" << fileName << "\"" << " ";
        }
    };

    // SPIR-V binaries generation.
    if (options.is_spirv_binaries_required)
    {
        // Compute.
        add_output_file(!options.pipeline_shaders.compute_shader.isEmpty(), "out.comp.spv=", kStrVkOfflineSpirvOutputFilenameCompute);

        if (options.pipeline_shaders.compute_shader.isEmpty() || is_spirv || is_pipe_file)
        {
            // Vertex.
            add_output_file(!options.pipeline_shaders.vertex_shader.isEmpty(), "out.vert.spv=", kStrVkOfflineSpirvOutputFilenameVertex);
            // Tessellation control.
            add_output_file(!options.pipeline_shaders.tessellation_control_shader.isEmpty(), "out.tesc.spv=", kStrVkOfflineSpirvOutputFilenameTessellationControl);
            // Tessellation evaluation.
            add_output_file(!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty(), "out.tese.spv=", kStrVkOfflineSpirvOutputFilenameTessellationEvaluation);
            // Geometry.
            add_output_file(!options.pipeline_shaders.geometry_shader.isEmpty(), "out.geom.spv=", kStrVkOfflineSpirvOutputFilenameGeometry);
            // Fragment.
            add_output_file(!options.pipeline_shaders.fragment_shader.isEmpty(), "out.frag.spv=", kStrVkOfflineSpirvOutputFilenameFragment);
        }
    }

    // AMD IL Binaries generation (for now we only support PAL IL).
    if (options.is_amd_pal_il_binaries_required)
    {
        // Compute.
        add_output_file(!options.pipeline_shaders.compute_shader.isEmpty(), "out.comp.palIl=", kStrPalIlOutputFilenameCompute);

        if (options.pipeline_shaders.compute_shader.isEmpty() || is_spirv)
        {
            // Vertex.
            add_output_file(!options.pipeline_shaders.vertex_shader.isEmpty(), "out.vert.palIl=", kStrPalIlOutputFilenameVert);
            // Tessellation control.
            add_output_file(!options.pipeline_shaders.tessellation_control_shader.isEmpty(), "out.tesc.palIl=", kStrPalIlOutputFilenameTessellationControl);
            // Tessellation evaluation.
            add_output_file(!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty(), "out.tese.palIl=", kStrPalIlOutputFilenameTessellationEvaluation);
            // Geometry.
            add_output_file(!options.pipeline_shaders.geometry_shader.isEmpty(), "out.geom.palIl=", kStrPalIlOutputFilenameGeometry);
            // Fragment.
            add_output_file(!options.pipeline_shaders.fragment_shader.isEmpty(), "out.frag.palIl=", kStrPalIlOutputFilenameFragment);
        }
    }

    // AMD IL disassembly generation (for now we only support PAL IL).
    if (options.is_amd_pal_disassembly_required)
    {
        // Compute.
        add_output_file(!options.pipeline_shaders.compute_shader.isEmpty(), "out.comp.palIlText=", options.pal_il_disassembly_output_files.compute_shader.asASCIICharArray());

        if (options.pipeline_shaders.compute_shader.isEmpty() || is_spirv || is_pipe_file)
        {
            // Vertex.
            add_output_file(!options.pipeline_shaders.vertex_shader.isEmpty(), "out.vert.palIlText=", options.pal_il_disassembly_output_files.vertex_shader.asASCIICharArray());
            // Tessellation control.
            add_output_file(!options.pipeline_shaders.tessellation_control_shader.isEmpty(), "out.tesc.palIlText=", options.pal_il_disassembly_output_files.tessellation_control_shader.asASCIICharArray());
            // Tessellation evaluation.
            add_output_file(!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty(), "out.tese.palIlText=", options.pal_il_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray());
            // Geometry.
            add_output_file(!options.pipeline_shaders.geometry_shader.isEmpty(), "out.geom.palIlText=", options.pal_il_disassembly_output_files.geometry_shader.asASCIICharArray());
            // Fragment.
            add_output_file(!options.pipeline_shaders.fragment_shader.isEmpty(), "out.frag.palIlText=", options.pal_il_disassembly_output_files.fragment_shader.asASCIICharArray());
        }
    }

    // AMD ISA binary generation.
    if (options.is_amd_isa_binaries_required)
    {
        // Compute.
        add_output_file(!options.pipeline_shaders.compute_shader.isEmpty(), "out.comp.isa=", options.isa_binary_output_files.compute_shader.asASCIICharArray());

        if (options.pipeline_shaders.compute_shader.isEmpty() || is_spirv || is_pipe_file)
        {
            // Vertex.
            add_output_file(!options.pipeline_shaders.vertex_shader.isEmpty(), "out.vert.isa=", options.isa_binary_output_files.vertex_shader.asASCIICharArray());
            // Tessellation control.
            add_output_file(!options.pipeline_shaders.tessellation_control_shader.isEmpty(), "out.tesc.isa=", options.isa_binary_output_files.tessellation_control_shader.asASCIICharArray());
            // Tessellation evaluation.
            add_output_file(!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty(), "out.tese.isa=", options.isa_binary_output_files.tessellation_evaluation_shader.asASCIICharArray());
            // Geometry.
            add_output_file(!options.pipeline_shaders.geometry_shader.isEmpty(), "out.geom.isa=", options.isa_binary_output_files.geometry_shader.asASCIICharArray());
            // Fragment.
            add_output_file(!options.pipeline_shaders.fragment_shader.isEmpty(), "out.frag.isa=", options.isa_binary_output_files.fragment_shader.asASCIICharArray());
        }
    }

    // Pipeline ELF binary generation.
    if (options.is_pipeline_binary_required)
    {
        add_output_file(!options.pipeline_binary.empty(), "out.pipeBin=", options.pipeline_binary);
    }

    // AMD ISA disassembly generation.
    if (options.is_amd_isa_disassembly_required)
    {
        // Compute.
        add_output_file(!options.pipeline_shaders.compute_shader.isEmpty(), "out.comp.isaText=", options.isa_disassembly_output_files.compute_shader.asASCIICharArray());

        if (options.pipeline_shaders.compute_shader.isEmpty() || is_spirv || is_pipe_file)
        {
            // Vertex.
            add_output_file(!options.pipeline_shaders.vertex_shader.isEmpty(), "out.vert.isaText=", options.isa_disassembly_output_files.vertex_shader.asASCIICharArray());
            // Tessellation control.
            add_output_file(!options.pipeline_shaders.tessellation_control_shader.isEmpty(), "out.tesc.isaText=", options.isa_disassembly_output_files.tessellation_control_shader.asASCIICharArray());
            // Tessellation evaluation.
            add_output_file(!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty(), "out.tese.isaText=", options.isa_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray());
            // Geometry.
            add_output_file(!options.pipeline_shaders.geometry_shader.isEmpty(), "out.geom.isaText=", options.isa_disassembly_output_files.geometry_shader.asASCIICharArray());
            // Fragment.
            add_output_file(!options.pipeline_shaders.fragment_shader.isEmpty(), "out.frag.isaText=", options.isa_disassembly_output_files.fragment_shader.asASCIICharArray());
        }
    }

    // Shader compiler statistics disassembly generation.
    if (options.is_stats_required)
    {
        // Compute.
        add_output_file(!options.pipeline_shaders.compute_shader.isEmpty(), "out.comp.isaInfo=", options.stats_output_files.compute_shader.asASCIICharArray());

        if (options.pipeline_shaders.compute_shader.isEmpty() || is_spirv || is_pipe_file)
        {
            // Vertex.
            add_output_file(!options.pipeline_shaders.vertex_shader.isEmpty(), "out.vert.isaInfo=", options.stats_output_files.vertex_shader.asASCIICharArray());
            // Tessellation control.
            add_output_file(!options.pipeline_shaders.tessellation_control_shader.isEmpty(), "out.tesc.isaInfo=", options.stats_output_files.tessellation_control_shader.asASCIICharArray());
            // Tessellation evaluation.
            add_output_file(!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty(), "out.tese.isaInfo=", options.stats_output_files.tessellation_evaluation_shader.asASCIICharArray());
            // Geometry.
            add_output_file(!options.pipeline_shaders.geometry_shader.isEmpty(), "out.geom.isaInfo=", options.stats_output_files.geometry_shader.asASCIICharArray());
            // Fragment.
            add_output_file(!options.pipeline_shaders.fragment_shader.isEmpty(), "out.frag.isaInfo=", options.stats_output_files.fragment_shader.asASCIICharArray());
        }
    }
}

static void MergeShaderOutput(const VkOfflineOptions& options, const gtString& disassembly_file_from, const gtString& disassembly_file_to,
    const gtString& stats_file_from, const gtString& stats_file_to, const gtString& binary_file_from, const gtString& binary_file_to)
{
    // Disassembly.
    if (options.is_amd_isa_disassembly_required)
    {
        std::string txt;
        bool is_disassembly_file_read = KcUtils::ReadTextFile(disassembly_file_from.asASCIICharArray(), txt, nullptr);
        assert(is_disassembly_file_read);
        assert(!disassembly_file_to.isEmpty());
        if (is_disassembly_file_read && !disassembly_file_to.isEmpty())
        {
            KcUtils::WriteTextFile(disassembly_file_to.asASCIICharArray(), txt, nullptr);
        }
    }

    // Statistics.
    if (options.is_stats_required)
    {
        std::string txt_stats;
        bool is_stats_file_read = KcUtils::ReadTextFile(stats_file_from.asASCIICharArray(), txt_stats, nullptr);
        assert(is_stats_file_read);
        assert(!stats_file_to.isEmpty());
        if (is_stats_file_read && !stats_file_to.isEmpty())
        {
            KcUtils::WriteTextFile(stats_file_to.asASCIICharArray(), txt_stats, nullptr);
        }
    }

    // Binaries.
    if (options.is_amd_isa_binaries_required)
    {
        std::vector<char> binary_content;
        bool is_bin_file_read = BeUtils::ReadBinaryFile(binary_file_from.asASCIICharArray(), binary_content);
        assert(is_bin_file_read);
        assert(!binary_file_to.isEmpty());
        if (is_bin_file_read && !binary_file_to.isEmpty())
        {
            KcUtils::WriteBinaryFile(binary_file_to.asASCIICharArray(), binary_content, nullptr);
        }
    }
}

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

beKA::beStatus BeProgramBuilderVkOffline::GetKernelIlText(const std::string& device, const std::string& kernel, std::string& il)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(il);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderVkOffline::GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(isa);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderVkOffline::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(analysis);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderVkOffline::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    (void)table;
    return beKA::kBeStatusInvalid;
}

// Checks if the required output files are generated by the amdspv.
// Only verifies the files requested in the "options.m_pipelineShaders" name list.
static bool VerifyAmdspvOutput(const VkOfflineOptions& options, const std::string& amdspv_gfx_ip)
{
    bool  ret = true;

    // For now, only perform input validation for pre Vega targets.
    // This should be updated to take into consideration shader merging
    // which may happen in Vega subsequent generations.
    if (!amdspv_gfx_ip.empty() && amdspv_gfx_ip[0] != '1' && amdspv_gfx_ip[0] != '9')
    {
        if (options.is_amd_isa_disassembly_required)
        {
            ret &= (options.pipeline_shaders.compute_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.compute_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.fragment_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.fragment_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.geometry_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.geometry_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.tessellation_control_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.vertex_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_disassembly_output_files.vertex_shader.asASCIICharArray()));
        }
        if (ret && options.is_amd_isa_binaries_required)
        {
            ret &= (options.pipeline_shaders.compute_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_binary_output_files.compute_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.fragment_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_binary_output_files.fragment_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.geometry_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_binary_output_files.geometry_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_binary_output_files.tessellation_control_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_binary_output_files.tessellation_evaluation_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.vertex_shader.isEmpty() || BeUtils::IsFilePresent(options.isa_binary_output_files.vertex_shader.asASCIICharArray()));
        }
        if (ret && options.is_spirv_binaries_required)
        {
            ret &= (options.pipeline_shaders.compute_shader.isEmpty() || BeUtils::IsFilePresent(kStrVkOfflineSpirvOutputFilenameCompute));
            ret &= (options.pipeline_shaders.fragment_shader.isEmpty() || BeUtils::IsFilePresent(kStrVkOfflineSpirvOutputFilenameFragment));
            ret &= (options.pipeline_shaders.geometry_shader.isEmpty() || BeUtils::IsFilePresent(kStrVkOfflineSpirvOutputFilenameGeometry));
            ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() || BeUtils::IsFilePresent(kStrVkOfflineSpirvOutputFilenameTessellationControl));
            ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() || BeUtils::IsFilePresent(kStrVkOfflineSpirvOutputFilenameTessellationEvaluation));
            ret &= (options.pipeline_shaders.vertex_shader.isEmpty() || BeUtils::IsFilePresent(kStrVkOfflineSpirvOutputFilenameVertex));
        }
        if (ret && options.is_amd_pal_il_binaries_required)
        {
            ret &= (options.pipeline_shaders.compute_shader.isEmpty() || BeUtils::IsFilePresent(kStrPalIlOutputFilenameCompute));
            ret &= (options.pipeline_shaders.fragment_shader.isEmpty() || BeUtils::IsFilePresent(kStrPalIlOutputFilenameFragment));
            ret &= (options.pipeline_shaders.geometry_shader.isEmpty() || BeUtils::IsFilePresent(kStrPalIlOutputFilenameGeometry));
            ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() || BeUtils::IsFilePresent(kStrPalIlOutputFilenameTessellationControl));
            ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() || BeUtils::IsFilePresent(kStrPalIlOutputFilenameTessellationEvaluation));
            ret &= (options.pipeline_shaders.vertex_shader.isEmpty() || BeUtils::IsFilePresent(kStrPalIlOutputFilenameVert));
        }
        if (ret && options.is_amd_pal_disassembly_required)
        {
            ret &= (options.pipeline_shaders.compute_shader.isEmpty() || BeUtils::IsFilePresent(options.pal_il_disassembly_output_files.compute_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.fragment_shader.isEmpty() || BeUtils::IsFilePresent(options.pal_il_disassembly_output_files.fragment_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.geometry_shader.isEmpty() || BeUtils::IsFilePresent(options.pal_il_disassembly_output_files.geometry_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() || BeUtils::IsFilePresent(options.pal_il_disassembly_output_files.tessellation_control_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() || BeUtils::IsFilePresent(options.pal_il_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.vertex_shader.isEmpty() || BeUtils::IsFilePresent(options.pal_il_disassembly_output_files.vertex_shader.asASCIICharArray()));
        }
        if (ret && options.is_stats_required)
        {
            ret &= (options.pipeline_shaders.compute_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.compute_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.fragment_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.fragment_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.geometry_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.geometry_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.tessellation_control_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.tessellation_evaluation_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.vertex_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.vertex_shader.asASCIICharArray()));
        }
    }
    else
    {
        if (!options.pipeline_shaders.geometry_shader.isEmpty() && options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() &&
            !BeUtils::IsFilePresent(options.isa_disassembly_output_files.geometry_shader.asASCIICharArray()))
        {
            // Geometry is present, but tessellation control is not present - geometry merged with vertex.
            MergeShaderOutput(options,
                options.isa_disassembly_output_files.vertex_shader,
                options.isa_disassembly_output_files.geometry_shader,
                options.stats_output_files.vertex_shader,
                options.stats_output_files.geometry_shader,
                options.isa_binary_output_files.vertex_shader,
                options.isa_binary_output_files.geometry_shader);
            std::cout << kStrInfoVulkanMergedShadersGeometryVertex << std::endl;
        }
        else if (!options.pipeline_shaders.geometry_shader.isEmpty() && !options.pipeline_shaders.tessellation_evaluation_shader.isEmpty())
        {
            // Geometry and tessellation evaluation both present - geometry merged with tessellation evaluation.
            MergeShaderOutput(options,
                options.isa_disassembly_output_files.tessellation_evaluation_shader,
                options.isa_disassembly_output_files.geometry_shader,
                options.stats_output_files.tessellation_evaluation_shader,
                options.stats_output_files.geometry_shader,
                options.isa_binary_output_files.tessellation_evaluation_shader,
                options.isa_binary_output_files.geometry_shader);
            std::cout << kStrInfoVulkanMergedShadersGeometryTessellationEvaluation << std::endl;
        }

        if (!options.pipeline_shaders.tessellation_control_shader.isEmpty() && !options.pipeline_shaders.vertex_shader.isEmpty())
        {
            // Tessellation control is present - tessellation control merged with vertex.
            MergeShaderOutput(options,
                options.isa_disassembly_output_files.vertex_shader,
                options.isa_disassembly_output_files.tessellation_control_shader,
                options.stats_output_files.vertex_shader,
                options.stats_output_files.tessellation_control_shader,
                options.isa_binary_output_files.vertex_shader,
                options.isa_binary_output_files.tessellation_control_shader);
            std::cout << kStrInfoVulkanMergedShadersTessellationControlVertex << std::endl;
        }
    }

    return ret;
}

beKA::beStatus BeProgramBuilderVkOffline::Compile(const VkOfflineOptions& vulkan_options, bool& cancel_signal, bool sshould_print_cmd, gtString& build_log)
{
    GT_UNREFERENCED_PARAMETER(cancel_signal);
    beKA::beStatus ret = beKA::kBeStatusGeneralFailed;
    build_log.makeEmpty();

    // Get amdspv's path.
    std::string amdspv_path;
    GetAmdspvPath(amdspv_path);

    AMDTDeviceInfoUtils* device_info = AMDTDeviceInfoUtils::Instance();
    if (device_info != nullptr)
    {
        // Numerical representation of the HW generation.
        std::string device_gfx_ip;

        // Convert the HW generation to the amdspv string.
        bool is_device_hw_gen_extracted = GetGfxIpForVulkan(device_info, vulkan_options, device_gfx_ip);
        if (is_device_hw_gen_extracted && !device_gfx_ip.empty())
        {
            // Build the command for invoking amdspv.
            std::stringstream cmd;
            cmd << amdspv_path;

            if (vulkan_options.optimization_level != -1)
            {
                cmd << " -O" << std::to_string(vulkan_options.optimization_level) << " ";
            }

            cmd << " -Dall -l -gfxip " << device_gfx_ip << " -set ";

            if ((ret = AddInputFileNames(vulkan_options, cmd)) == beKA::kBeStatusSuccess)
            {
                AddOutputFileNames(vulkan_options, cmd);

                // Redirect build log to a temporary file.
                const gtString kAmdspvTmpOutputFile = L"amdspvTempFile.txt";
                osFilePath tmp_file_path(osFilePath::OS_TEMP_DIRECTORY);
                tmp_file_path.setFileName(kAmdspvTmpOutputFile);

                // Delete the log file if it already exists.
                if (tmp_file_path.exists())
                {
                    osFile tmp_log_file(tmp_file_path);
                    tmp_log_file.deleteFile();
                }

                cmd << "out.glslLog=\"" << tmp_file_path.asString().asASCIICharArray() << "\" ";

                // No default output (only generate the output files that we explicitly specified).
                cmd << "defaultOutput=0";

                // Disable validate.
                cmd << " -set val=0 ";

                if (!vulkan_options.pipe_file.empty())
                {
                    // Append the .pipe file name (no command line option is needed since the extension is .pipe).
                    cmd << "\"" << vulkan_options.pipe_file << "\" ";
                }

                // Launch amdspv.
                gtString amdspv_output;
                BeUtils::PrintCmdLine(cmd.str(), sshould_print_cmd);
                bool is_launch_success = osExecAndGrabOutput(cmd.str().c_str(), cancel_signal, amdspv_output);
                if (is_launch_success)
                {
                    // This is how amdspv signals success.
                    const gtString kAmdspvTokenSuccess = L"SUCCESS!";

                    // Check if the output files were generated and amdspv returned "success".
                    if (amdspv_output.find(kAmdspvTokenSuccess) == std::string::npos)
                    {
                        ret = beKA::kBeStatusVulkanAmdspvCompilationFailure;

                        // Read the build log.
                        if (tmp_file_path.exists())
                        {
                            // Read the build log.
                            gtString compiler_output;
                            std::ifstream file(tmp_file_path.asString().asASCIICharArray());
                            std::string tmp_cmd_output((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                            build_log << tmp_cmd_output.c_str();

                            // Delete the temporary file.
                            osFile file_to_delete(tmp_file_path);
                            file_to_delete.deleteFile();
                        }

                        // Let's end the build log with the error that was provided by the backend.
                        if (!amdspv_output.isEmpty())
                        {
                            build_log << "Error: " << amdspv_output << L"\n";
                        }
                    }
                    else if (!VerifyAmdspvOutput(vulkan_options, device_gfx_ip))
                    {
                        ret = beKA::kBeStatusFailedOutputVerification;
                    }
                    else
                    {
                        ret = beKA::kBeStatusSuccess;

                        // Delete the ISA binaries if they are not required.
                        if (!vulkan_options.is_amd_isa_binaries_required)
                        {
                            BeUtils::DeleteOutputFiles(vulkan_options.isa_binary_output_files);
                        }
                    }
                }
                else
                {
                    ret = beKA::kBeStatusVulkanAmdspvLaunchFailure;
                }
            }
        }
        else
        {
            ret = beKA::kBeStatusOpenglUnknownHwFamily;
        }
    }

    return ret;
}

bool BeProgramBuilderVkOffline::GetVulkanVersion(gtString& vk_version) const
{
    const wchar_t* kBeStrVulkanVersion = L"Based on Vulkan 1.0 Specification.";
    vk_version = kBeStrVulkanVersion;
    return true;
}

bool BeProgramBuilderVkOffline::GetSupportedDevices(std::set<std::string>& device_list)
{
    std::vector<GDT_GfxCardInfo> tmp_card_list;
    bool ret = BeUtils::GetAllGraphicsCards(tmp_card_list, device_list);
    return ret;
}
