//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for rga backend progam builder vulkan offline class.
//=============================================================================

// C++.
#include <sstream>
#include <cassert>

// Infra.
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_os_wrappers/Include/osProcess.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder_vk_offline.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Device info.
#include "DeviceInfoUtils.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// Targets of Amdllpc gfxip and corresponding DeviceInfo names.
static const std::map<std::string, std::string> kVkAmdllpcTargetsToDeviceInfoTargets = {
    {"gfx900", "9.0.0"},   
    {"gfx902", "9.0.2"},   
    {"gfx904", "9.0.4"},   
    {"gfx906", "9.0.6"},   
    {"gfx90c", "9.0.12"},  
    {"gfx1010", "10.1.0"},
    {"gfx1011", "10.1.1"}, 
    {"gfx1012", "10.1.2"}, 
    {"gfx1030", "10.3.0"}, 
    {"gfx1031", "10.3.1"}, 
    {"gfx1032", "10.3.2"}, 
    {"gfx1034", "10.3.4"},
    {"gfx1035", "10.3.5"}, 
    {"gfx1100", "11.0.0"}, 
    {"gfx1101", "11.0.1"}, 
    {"gfx1102", "11.0.2"}, 
    {"gfx1103", "11.0.3"}, 
    {"gfx1150", "11.5.0"},
    {"gfx1151", "11.5.1"}, 
    {"gfx1152", "11.5.2"}, 
    {"gfx1200", "12.0.0"}, 
    {"gfx1201", "12.0.1"}};
// gfx110x is not supported by amdllpc as of 07/11/2023.


static bool GetAmdllpcPath(std::string& amdllpc_path)
{
#ifdef __linux
    amdllpc_path = "amdllpc";
#elif _WIN64
    amdllpc_path = "utils\\amdllpc.exe";
#elif _WIN32
    amdllpc_path = "x86\\amdllpc.exe";
#endif
    return true;
}

static bool GetAmdllpcGfxIpForVulkan(const VkOfflineOptions& vulkan_options, std::string& gfx_ip_str)
{
    bool ret = false;
    gfx_ip_str.clear();

    auto itr = kVkAmdllpcTargetsToDeviceInfoTargets.find(vulkan_options.target_device_name);
    if (itr != kVkAmdllpcTargetsToDeviceInfoTargets.end())
    {
        gfx_ip_str = itr->second;
        ret        = true;
    }
    return ret;
}

static beKA::beStatus AddAmdllpcInputFileNames(const VkOfflineOptions& options, std::stringstream& cmd)
{
    beKA::beStatus status = beKA::kBeStatusSuccess;

    // If .pipe input, we don't need to add any other file.
    bool is_pipe_input = !options.pipe_file.empty();

    // Indicates that a stage-less input file name was provided.
    bool is_non_stage_input = false;

    // Indicates that some of stage-specific file names was provided (--frag, --vert, etc.).
    bool is_stage_input = false;

    if (options.mode == beKA::kModeVkOfflineSpv || options.mode == beKA::kModeVkOfflineSpvTxt)
    {
        if (!options.stageless_input_file.empty())
        {
            cmd << "\"" << options.stageless_input_file << "\" ";
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
            cmd << "\"" << options.pipeline_shaders.vertex_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Tessellation control shader.
        if (!options.pipeline_shaders.tessellation_control_shader.isEmpty())
        {
            cmd << "\"" << options.pipeline_shaders.tessellation_control_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Tessellation evaluation shader.
        if (!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty())
        {
            cmd << "\"" << options.pipeline_shaders.tessellation_evaluation_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Geometry shader.
        if (!options.pipeline_shaders.geometry_shader.isEmpty())
        {
            cmd << "\"" << options.pipeline_shaders.geometry_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Fragment shader.
        if (!options.pipeline_shaders.fragment_shader.isEmpty())
        {
            cmd << "\"" << options.pipeline_shaders.fragment_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Mesh shader.
        if (!options.pipeline_shaders.mesh_shader.isEmpty())
        {
            cmd << "\"" << options.pipeline_shaders.mesh_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }
        
        // Task shader.
        if (!options.pipeline_shaders.task_shader.isEmpty())
        {
            cmd << "\"" << options.pipeline_shaders.task_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }
    }
    else
    {
        // Compute shader.
        cmd << "\"" << options.pipeline_shaders.compute_shader.asASCIICharArray() << "\" ";
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

static void AddAmdllpcOutputFileNames(const VkOfflineOptions& options, std::stringstream& cmd)
{
    bool is_spirv = (options.mode == beKA::kModeVkOfflineSpv || options.mode == beKA::kModeVkOfflineSpvTxt);

    bool is_pipe_file = !is_spirv && !options.pipe_file.empty();

    auto add_output_file = [&](bool flag, const std::string& option, const std::string& fileName) {
        if (flag || is_spirv || is_pipe_file)
        {
            cmd << option << "\"" << fileName << "\""
                << " ";
        }
    };

    // Pipeline ELF binary generation.
    if (options.is_pipeline_binary_required)
    {
        add_output_file(!options.pipeline_binary.empty(), " -o=", options.pipeline_binary);
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

beKA::beStatus BeProgramBuilderVkOffline::CompileWithAmdllpc(const VkOfflineOptions& vulkan_options,
                                                             bool&                   cancel_signal,
                                                             bool                    should_print_cmd,
                                                             gtString&               build_log)
{
    GT_UNREFERENCED_PARAMETER(cancel_signal);
    beKA::beStatus ret = beKA::kBeStatusGeneralFailed;
    build_log.makeEmpty();

    // Get amdllpc's path.
    std::string amdllpc_path;
    GetAmdllpcPath(amdllpc_path);

    std::string device_gfx_ip;
    bool is_device_valid = GetAmdllpcGfxIpForVulkan(vulkan_options, device_gfx_ip);
    if (is_device_valid && !device_gfx_ip.empty())
    {
        // Build the command for invoking amdspv.
        std::stringstream cmd;
        cmd << amdllpc_path;

        // amdllpc.exe -v --gfxip=11 C:\vkoffline\bloom\bloom1_vert.spv --log-file-outs=log.txt -o out.bin 
        cmd << " -v";
        
        cmd << " --include-llvm-ir";

        cmd << " --auto-layout-desc";

        // Redirect build log to a temporary file.
        const gtString kAmdllpcTmpOutputFile = L"amdllpcTempFile.txt";
        osFilePath     tmp_file_path(osFilePath::OS_TEMP_DIRECTORY);
        tmp_file_path.setFileName(kAmdllpcTmpOutputFile);

        // Delete the log file if it already exists.
        if (tmp_file_path.exists())
        {
            osFile tmp_log_file(tmp_file_path);
            tmp_log_file.deleteFile();
        }

        cmd << " --log-file-outs=\"" << tmp_file_path.asString().asASCIICharArray() << "\" ";

        AddAmdllpcOutputFileNames(vulkan_options, cmd);

        cmd << "--gfxip=" << device_gfx_ip << " ";

        if ((ret = AddAmdllpcInputFileNames(vulkan_options, cmd)) == beKA::kBeStatusSuccess)
        {        

            if (!vulkan_options.pipe_file.empty())
            {
                // Append the .pipe file name (no command line option is needed since the extension is .pipe).
                cmd << "\"" << vulkan_options.pipe_file << "\" ";
            }

            // Launch amdllpc.
            gtString amdllpc_output;
            std::string cmdStr = cmd.str();
            BeUtils::PrintCmdLine(cmdStr, should_print_cmd);
            bool is_launch_success = osExecAndGrabOutput(cmd.str().c_str(), cancel_signal, amdllpc_output);
            if (is_launch_success)
            {
                // This is how amdspv signals success.
                const gtString kAmdllpcTokenSuccess = L"AMDLLPC SUCCESS";

                // Check if the output files were generated and amdllpc returned "success".
                if (amdllpc_output.find(kAmdllpcTokenSuccess) == std::string::npos)
                {
                    ret = beKA::kBeStatusVulkanAmdllpcCompilationFailure;

                    // Read the build log.
                    if (tmp_file_path.exists())
                    {
                        // Read the build log.
                        gtString      compiler_output;
                        std::ifstream file(tmp_file_path.asString().asASCIICharArray());
                        std::string   tmp_cmd_output((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                        build_log << tmp_cmd_output.c_str();

                        // Delete the temporary file.
                        osFile file_to_delete(tmp_file_path);
                        file_to_delete.deleteFile();
                    }

                    // Let's end the build log with the error that was provided by the backend.
                    if (!amdllpc_output.isEmpty())
                    {
                        build_log << "Error: " << amdllpc_output << L"\n";
                    }
                }
                else
                {
                    ret = beKA::kBeStatusSuccess;
                }
            }
            else
            {
                ret = beKA::kBeStatusVulkanAmdllpcLaunchFailure;
            }
        }
    }
    else
    {
        ret = beKA::kBeStatusOpenglUnknownHwFamily;
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
