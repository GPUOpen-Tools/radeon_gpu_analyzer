//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++
#include <cassert>
#include <stdlib.h>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "AMDTOSWrappers/Include/osFilePath.h"
#include "AMDTOSWrappers/Include/osDirectory.h"
#include "AMDTOSWrappers/Include/osApplication.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"
#include "radeon_gpu_analyzer_backend/be_string_constants.h"
#include "radeon_gpu_analyzer_backend/be_data_types.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"
#include "source/common/rg_log.h"
#include "source/common/rga_cli_defs.h"

using namespace beKA;

// Constants.

// Glslang option: output file.
static const std::string  kStrGlslangOptOutput       = "-o";

// Glslang option: SPIR-V binary output file.
static const std::string  kStrGlslangOptSpirvOutput = "-V";

// Glslang option: Input file is an HLSL shader.
static const std::string  kStrGlslangOptHlslInput   = "-D";

// Glslang option: Macro (yes, it is the same as HLSL shader).
// For macros, the usage a different as an argument is being
// appended to the command line switch of glslang.
static const std::string  kStrGlslangOptMacroDefine = "-D";

// Glslang option: explicitly specify shader stage.
static const std::string  kStrGlslangOptShaderStage = "-S";

// Glslang option: Include directory.
static const std::string  kStrGlslangOptIncludeDir = "-I";

// Glslang option: Preprocess input GLSL/HLSL file to stdout.
static const std::string  kStrGlslangOptPreprocess   = "-E";

// Glslang option: Pipeline stage.
static const std::string  kStrGlslangOptGlslangStage        = "-S";

// Glslang options: Pipeline stage names.
static const std::string  kStrGlslangOptStageVert   = "vert";
static const std::string  kStrGlslangOptStageTessellationControl   = "tesc";
static const std::string  kStrGlslangOptStageTessellationEvaluation   = "tese";
static const std::string  kStrGlslangOptStageGeometry   = "geom";
static const std::string  kStrGlslangOptStageFragment   = "frag";
static const std::string  kStrGlslangOptStageCompute   = "comp";

// Info messages.
static const char* kStrGlslangOptDebugInfoBegin = "*** Loader debug info - BEGIN ***";
static const char* kStrGlslangOptDebugInfoEnd = "*** Loader debug info - END ***";

// A container for all valid glslang extensions for GLSL automatic stage detection.
static const vector<std::string> kValidGlslangGlslExtensions =
{
    kStrGlslangOptStageVert,
    kStrGlslangOptStageTessellationControl,
    kStrGlslangOptStageTessellationEvaluation,
    kStrGlslangOptStageGeometry,
    kStrGlslangOptStageFragment,
    kStrGlslangOptStageCompute
};

// SPIR-V disassembler option: output file.
static const std::string  kStrSpvDisOptOutput = "-o";

// VulkanBackend options: input spv files.
static const std::array<std::string, BePipelineStage::kCount>
kStrVulkanBackendOptStageInputFile =
{
    "--vert",
    "--tesc",
    "--tese",
    "--geom",
    "--frag",
    "--comp"
};

// VulkanBackend options: output ISA disassembly files.
static const std::array<std::string, BePipelineStage::kCount>
kVulkanBackendOptStageIsaFile =
{
    "--vert-isa",
    "--tesc-isa",
    "--tese-isa",
    "--geom-isa",
    "--frag-isa",
    "--comp-isa"
};

// VulkanBackend options: output statistics files.
static const std::array<std::string, BePipelineStage::kCount>
kVulkanBackendOptStageStatsFile =
{
    "--vert-stats",
    "--tesc-stats",
    "--tese-stats",
    "--geom-stats",
    "--frag-stats",
    "--comp-stats"
};

// VulkanBackend options: output binary file.
static const std::string  kVulkanBackendOptBinFile = "--bin";

// VulkanBackend options: input pipeline object file.
static const std::string  kVulkanBackendOptPsoFile = "--pso";

// VulkanBackend options: input alternative ICD path.
static const std::string  kVulkanBackendOptIcdPath = "--icd";

// VulkanBackend options: value for VK_LOADER_DEBUG environment variable.
static const std::string  kVulkanBackendOptVkLoaderDebug = "--loader-debug";

// VulkanBackend options: list target GPUs.
static const std::string  kVulkanBackendOptListTargets = "--list-targets";

// VulkanBackend options: list physical GPU adapters.
static const std::string  kVulkanBackendOptListAdapters = "--list-adapters";

// VulkanBackend options: enable Vulkan validation layers.
static const std::string  kVulkanBackendOptEnableLayers = "--enable-layers";

// VulkanBackend options: path to the validation output text file.
static const std::string  kVulkanBackendOptLayersFile = "--layers-file";

// VulkanBackend options: target GPU.
static const std::string  kVulkanBackendOptTarget = "--target";

// Copy the Vulkan Validation layers info from temp file ("tempInfoFile") to the Log file and user-provided validation info file ("outputFile").
// Delete the temp info file after copying its content.
static void CopyValidatioInfo(const std::string& temp_info_file, const std::string& output_file)
{
    static const char* kStrWarningFailedExtractValidationInfo = "<stdout>";

    bool result = false;
    std::string info;
    if ((result = KcUtils::ReadTextFile(temp_info_file, info, nullptr)) == true)
    {
        RgLog::file << info << std::endl;
        if (output_file == kStrCliVkValidationInfoStdout)
        {
            RgLog::stdOut << std::endl << info << std::endl;
        }
        else
        {
            result = KcUtils::WriteTextFile(output_file, info, nullptr);
        }
    }

    if (!KcUtils::DeleteFile(temp_info_file))
    {
        result = false;
    }

    if (!result)
    {
        RgLog::stdErr << kStrWarningFailedExtractValidationInfo << std::endl;
    }
}

// Construct command line options for Vulkan Backend.
static std::string ConstructVulkanBackendOptions(const std::string& loader_debug,
    const BeVkPipelineFiles& spv_files,
    const BeVkPipelineFiles& isa_files,
    const BeVkPipelineFiles& stats_files,
    const std::string& bin_file,
    const std::string& pso_file,
    const std::string& icd_file,
    const std::string& validation_output,
    const std::string& device)
{
    std::stringstream opts;

    // Add target option.
    if (!device.empty())
    {
        opts << kVulkanBackendOptTarget << " " << device;
    }

    // Add per-stage input & output files names.
    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
    {
        if (!spv_files[stage].empty())
        {
            assert(!isa_files[stage].empty() && !stats_files[stage].empty());
            if (!isa_files[stage].empty() && !stats_files[stage].empty())
            {
                opts << " " << kStrVulkanBackendOptStageInputFile[stage] << " " << KcUtils::Quote(spv_files[stage]) << " "
                    << kVulkanBackendOptStageIsaFile[stage] << " " << KcUtils::Quote(isa_files[stage]) << " "
                    << kVulkanBackendOptStageStatsFile[stage] << " " << KcUtils::Quote(stats_files[stage]) << " ";
            }
        }
    }

    // Output binary file name.
    if (!bin_file.empty())
    {
        opts << " " << kVulkanBackendOptBinFile << " " << KcUtils::Quote(bin_file);
    }

    // Pipeline state file.
    if (!pso_file.empty())
    {
        opts << " " << kVulkanBackendOptPsoFile << " " << KcUtils::Quote(pso_file);
    }

    // Alternative ICD full path.
    if (!icd_file.empty())
    {
        opts << " " << kVulkanBackendOptIcdPath << " " << KcUtils::Quote(icd_file);
    }

    // Value for VK_LOADER_DEBUG.
    if (!loader_debug.empty())
    {
        opts << " " << kVulkanBackendOptVkLoaderDebug << " " << KcUtils::Quote(loader_debug);
    }

    // Add validation enabling option if required.
    if (!validation_output.empty())
    {
        opts << " " << kVulkanBackendOptEnableLayers;
        opts << " " << kVulkanBackendOptLayersFile << " " << validation_output;
    }

    return opts.str();
}

// Construct command line options for SPIR-V disassembler.
static std::string ConstructSpvDisOptions(const std::string& spv_filename, const std::string& spv_dis_filename)
{
    std::stringstream opts;

    if (!spv_filename.empty())
    {
        if (!spv_dis_filename.empty())
        {
            opts << kStrSpvDisOptOutput << " " << KcUtils::Quote(spv_dis_filename);
        }
        opts << " " << KcUtils::Quote(spv_filename);
    }

    return opts.str();
}

// Construct command line options for SPIR-V assembler.
static std::string ConstructSpvAsmOptions(const std::string& spv_txt_filename, const std::string& spv_filename)
{
    std::stringstream opts;

    if (!spv_txt_filename.empty())
    {
        if (!spv_filename.empty())
        {
            opts << kStrSpvDisOptOutput << " " << KcUtils::Quote(spv_filename);
        }
        opts << " " << KcUtils::Quote(spv_txt_filename);
    }

    return opts.str();
}

// Construct command line options for Glslang compiler.
static std::string ConstructGlslangOptions(const Config& config, const std::string& src_filename,
    const std::string& spv_filename, size_t stage, bool is_hlsl, bool is_preprocess = false)
{
    std::stringstream opts;

    // Append any additional options from the user.
    if (!config.glslang_opt.empty())
    {
        // Unwrap the argument from the token.
        std::string fixed_options = config.glslang_opt;
        fixed_options.erase(std::remove(fixed_options.begin(),
            fixed_options.end(), kStrCliOptGlslangToken), fixed_options.end());
        opts << fixed_options;
    }

    if (is_hlsl)
    {
        opts << " " << kStrGlslangOptHlslInput;
    }

    if (is_preprocess)
    {
        opts << " " << kStrGlslangOptPreprocess;

        // Glslang preprocessor requires specifying the pipeline stage for some reason.
        // Always use "vert" since preprocessor is stage-agnostic.
        opts << " " << kStrGlslangOptGlslangStage << " " << kStrGlslangOptStageVert;
    }
    else
    {
        // Add the switches for any given include paths.
        if (!config.include_path.empty())
        {
            for (const std::string& includePath : config.include_path)
            {
                opts << " " << kStrGlslangOptIncludeDir << KcUtils::Quote(includePath) << " ";
            }
        }

        // Add the switches for any given macro/define directives..
        if (!config.defines.empty())
        {
            for (const std::string& givenMacro : config.defines)
            {
                opts << " " << kStrGlslangOptMacroDefine << givenMacro << " ";
            }
        }

        // Get the file extension.
        std::string ext = BeUtils::GetFileExtension(src_filename);

        // If the file extension is not one of the "default" file extensions:
        // vert, tesc, tese, geom, frag, comp, we need to explicitly specify for glslang the stage.
        if (std::find(kValidGlslangGlslExtensions.cbegin(),
            kValidGlslangGlslExtensions.cend(), ext) == kValidGlslangGlslExtensions.cend())
        {
            assert(stage < kValidGlslangGlslExtensions.size());
            if(stage < kValidGlslangGlslExtensions.size())
            {
                opts << " " << kStrGlslangOptShaderStage << " " <<
                    kValidGlslangGlslExtensions[stage] << " ";
            }
        }

        opts << " " << kStrGlslangOptSpirvOutput;
        opts << " " << kStrGlslangOptOutput << " " << KcUtils::Quote(spv_filename);
    }

    opts << " " << KcUtils::Quote(src_filename);

    return opts.str();
}

// Check if ISA disassembly and statistics files are not empty for corresponding input spv files.
static bool VerifyOutputFiles(const BeVkPipelineFiles& spv_files,
    const BeVkPipelineFiles& isa_files,
    const BeVkPipelineFiles& stats_files)
{
    bool result = true;
    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
    {
        if (!spv_files[stage].empty())
        {
            if (!KcUtils::FileNotEmpty(isa_files[stage]) || !KcUtils::FileNotEmpty(stats_files[stage]))
            {
                result = false;
                break;
            }
        }
    }

    return result;
}

beKA::beStatus beProgramBuilderVulkan::GetVulkanDriverTargetGPUs(const std::string& loader_debug, const std::string& icd_file, std::set<std::string>& target_gpus,
    bool should_print_cmd, std::string& errText)
{
    std::string std_out_text, std_err_text;
    std::string opts = kVulkanBackendOptListTargets;

    // Alternative ICD library path.
    if (!icd_file.empty())
    {
        opts += (" " + kVulkanBackendOptIcdPath + " " + KcUtils::Quote(icd_file));
    }

    // Launch the VulkanBacked and get the list of devices.
    beStatus status = InvokeVulkanBackend(opts, should_print_cmd, std_out_text, std_err_text);

    // If the VulkanBackend provided any warning/errors, display them.
    if (!std_err_text.empty())
    {
        RgLog::stdOut << std_err_text << std::endl;
    }

    // Process the target list.
    // The format of device list returned by VulkanBackend:
    // BONAIRE:gfx700
    // CARRIZO:gfx801
    // FIJI:gfx803:gfx804
    // ...

    if (status == kBeStatusSuccess)
    {
        size_t start = 0, end = 0;
        std::string devices_str = KcUtils::ToLower(std_out_text);

        while (start < devices_str.size() && (end = devices_str.find_first_of(":\n", start)) != std::string::npos)
        {
            target_gpus.insert(devices_str.substr(start, end - start));
            start = end + 1;
        }
    }

    return status;
}

beStatus beProgramBuilderVulkan::GetPhysicalGPUs(const std::string& icd_file, std::vector<BeVkPhysAdapterInfo>& gpu_info,
    bool should_print_cmd, std::string& error_text)
{
    // The format of physical adapter list returned by VulkanBackend:
    //
    // Adapter 0:
    //     Name: Radeon(TM) RX 480 Graphics
    //     Vulkan driver version : 2.0.0
    //     Supported Vulkan API version : 1.1.77

    std::string std_out_text, std_err_text;
    std::string opts = kVulkanBackendOptListAdapters;

    // Alternative ICD library path.
    if (!icd_file.empty())
    {
        opts += (" " + kVulkanBackendOptIcdPath + " " + KcUtils::Quote(icd_file));
    }

    // Invoke "VulkanBackend --list-adapters" to get the list of physical adapters installed on the system.
    beStatus status = InvokeVulkanBackend(opts, should_print_cmd, std_out_text, std_err_text);
    if (status == kBeStatusSuccess)
    {
        error_text = std_err_text;
        std::stringstream out_stream(std_out_text);
        std::string line;
        uint32_t id = 0;
        size_t offset = 0;

        // Parse the VulkanBackend output.
        while (std::getline(out_stream, line))
        {
            BeVkPhysAdapterInfo info;
            if (line.find(kStrCliVkBackendStrAdapter) == 0)
            {
                info.id = id++;
                if (std::getline(out_stream, line) && (offset = line.find(kStrCliVkBackendStrAdapterName)) != std::string::npos)
                {
                    info.name = line.substr(offset + kStrCliVkBackendStrAdapterName.size());
                }
                if (std::getline(out_stream, line) && (offset = line.find(kStrCliVkBackendStrAdapterDriver)) != std::string::npos)
                {
                    info.vk_driver_version = line.substr(offset + kStrCliVkBackendStrAdapterDriver.size());
                }
                if (std::getline(out_stream, line) && (offset = line.find(kStrCliVkBackendStrAdapterVulkan)) != std::string::npos)
                {
                    info.vk_api_version = line.substr(offset + kStrCliVkBackendStrAdapterVulkan.size());
                }
                gpu_info.push_back(info);
            }
        }
    }

    return status;
}

beKA::beStatus beProgramBuilderVulkan::CompileSrcToSpirvBinary(const Config& config,
    const std::string& src_file,
    const std::string& spv_file,
    BePipelineStage stage,
    bool is_hlsl,
    std::string& error_text)
{
    beStatus status = kBeStatusVulkanEmptyInputFile;
    std::string out_text;

    if (!src_file.empty())
    {
        std::string glslang_opts = ConstructGlslangOptions(config, src_file, spv_file, stage, is_hlsl);
        assert(!glslang_opts.empty());
        if (!glslang_opts.empty())
        {
            status = InvokeGlslang(config.compiler_bin_path, glslang_opts,
                config.print_process_cmd_line, out_text, error_text);
        }
    }

    return status;
}

beStatus beProgramBuilderVulkan::InvokeGlslang(const std::string& glslang_bin_dir, const std::string& cmd_line_options,
    bool should_print_cmd, std::string& out_text, std::string& error_text)
{
    osFilePath glslang_exec;
    long exitCode = 0;

    // Use the glslang folder provided by user if it's not empty.
    // Otherwise, use the default location.
    if (!glslang_bin_dir.empty())
    {
        gtString bin_folder;
        bin_folder << glslang_bin_dir.c_str();
        glslang_exec.setFileDirectory(bin_folder);
    }
    else
    {
        osGetCurrentApplicationPath(glslang_exec, false);
        glslang_exec.appendSubDirectory(kGlslangRootDir);
    }
    glslang_exec.setFileName(kGlslangExecutable);
    glslang_exec.setFileExtension(kGlslangExecutableExtension);

    // Clear the error message buffer.
    error_text.clear();

    KcUtils::ProcessStatus  status = KcUtils::LaunchProcess(glslang_exec.asString().asASCIICharArray(),
        cmd_line_options,
        "",
        kProcessWaitInfinite,
        should_print_cmd,
        out_text,
        error_text,
        exitCode);

    // If the output was streamed to stdout, grab it from there.
    if (error_text.empty() && !out_text.empty())
    {
        error_text = out_text;
    }

    return (status == KcUtils::ProcessStatus::kSuccess ? kBeStatusSuccess : kBeStatusVulkanGlslangLaunchFailed);
}

beStatus beProgramBuilderVulkan::InvokeSpvTool(BeVulkanSpirvTool tool, const std::string& spv_tools_bin_dir, const std::string& cmd_line_options,
    bool should_print_cmd, std::string& out_msg, std::string& error_msg)
{
    osFilePath spv_dis_exec;
    long exit_code = 0;

    if (!spv_tools_bin_dir.empty())
    {
        gtString bin_folder;
        bin_folder << spv_tools_bin_dir.c_str();
        spv_dis_exec.setFileDirectory(bin_folder);
    }
    else
    {
        osGetCurrentApplicationPath(spv_dis_exec, false);
        spv_dis_exec.appendSubDirectory(kGlslangRootDir);
    }

    const gtString spv_tool_exec_name = (tool == BeVulkanSpirvTool::kAssembler ? kSpirvAsExecutable :
        tool == BeVulkanSpirvTool::kDisassembler ? kSpirvDisExecutable  :
        L"");

    spv_dis_exec.setFileName(spv_tool_exec_name);
    spv_dis_exec.setFileExtension(kVulkanBackendExecutableExtension);

    KcUtils::ProcessStatus  status = KcUtils::LaunchProcess(spv_dis_exec.asString().asASCIICharArray(),
        cmd_line_options,
        "",
        kProcessWaitInfinite,
        should_print_cmd,
        out_msg,
        error_msg,
        exit_code);

    return (status == KcUtils::ProcessStatus::kSuccess ? kBeStatusSuccess : kBeStatusVulkanSpvToolLaunchFailed);
}

beStatus beProgramBuilderVulkan::InvokeVulkanBackend(const std::string& cmd_line_options, bool should_print_cmd,
    std::string& out_text, std::string& error_text)
{
    osFilePath vk_backend_exec;
    long exit_code = 0;

    // Construct the path to the VulkanBackend executable.
    osGetCurrentApplicationPath(vk_backend_exec, false);
    vk_backend_exec.appendSubDirectory(kVulkanBackendRootDir);
    vk_backend_exec.setFileName(kVulkanBackendExecutable);
    vk_backend_exec.setFileExtension(kVulkanBackendExecutableExtension);

    KcUtils::ProcessStatus status = KcUtils::LaunchProcess(vk_backend_exec.asString().asASCIICharArray(),
        cmd_line_options,
        "",
        kProcessWaitInfinite,
        should_print_cmd,
        out_text,
        error_text,
        exit_code);

    return (status == KcUtils::ProcessStatus::kSuccess ? kBeStatusSuccess : kBeStatusVulkanBackendLaunchFailed);
}

beKA::beStatus beProgramBuilderVulkan::CompileSpirv(const std::string& loader_debug,
    const BeVkPipelineFiles& spirv_files,
    const BeVkPipelineFiles& isa_files,
    const BeVkPipelineFiles& stats_files,
    const std::string& bin_file,
    const std::string& pso_file,
    const std::string& icd_file,
    const std::string& validation_output,
    const std::string& validation_output_redirection,
    const std::string& device,
    bool should_print_cmd,
    std::string& error_msg)
{
    beStatus  status = kBeStatusVulkanBackendLaunchFailed;
    std::string std_out_text, std_err_text;

    // Construct the command for invoking the Vulkan backend.
    std::string opts = ConstructVulkanBackendOptions(loader_debug, spirv_files, isa_files, stats_files, bin_file, pso_file, icd_file, validation_output, device);
    assert(!opts.empty());
    if (!opts.empty())
    {
        status = InvokeVulkanBackend(opts, should_print_cmd, std_out_text, std_err_text);

        // Check if some output files have not been generated for some reason.
        if (status == kBeStatusSuccess && !VerifyOutputFiles(spirv_files, isa_files, stats_files))
        {
            status = kBeStatusVulkanBackendCompileFailed;
            error_msg = std_err_text;
        }
        else if (!loader_debug.empty())
        {
            RgLog::stdOut << std::endl << kStrGlslangOptDebugInfoBegin << std::endl <<
                std::endl << std_err_text << std::endl << kStrGlslangOptDebugInfoEnd << std::endl;
        }

        // Print the Vulkan backend's output.
        if (!std_out_text.empty())
        {
            RgLog::stdOut << std::endl << std_out_text << std::endl;
        }

        // Dump the Vulkan validation info to the output file/stdout and log file.
        if (!validation_output_redirection.empty())
        {
            CopyValidatioInfo(validation_output, validation_output_redirection);
        }
    }

    return status;
}

beStatus beProgramBuilderVulkan::DisassembleSpv(const std::string& spv_tools_bin_dir, const std::string& spv_file_path,
    const std::string& spv_dis_file_path, bool printCmd, std::string& error_msg)
{
    beStatus status = kBeStatusVulkanSpvDisasmFailed;
    const std::string& opts = ConstructSpvDisOptions(spv_file_path, spv_dis_file_path);
    if (!opts.empty())
    {
        std::string out_msg;
        status = InvokeSpvTool(BeVulkanSpirvTool::kDisassembler, spv_tools_bin_dir, opts, printCmd, out_msg, error_msg);

        // Check if the spv-dis has generated expected output file.
        if (status == kBeStatusSuccess)
        {
            // Dump disassembly output to the stdout if no output file name is specified.
            if (spv_dis_file_path.empty())
            {
                RgLog::stdOut << out_msg << std::endl;
            }
            else if (!KcUtils::FileNotEmpty(spv_dis_file_path))
            {
                status = kBeStatusVulkanSpvDisasmFailed;
            }
        }
    }

    return status;
}

beStatus beProgramBuilderVulkan::AssembleSpv(const std::string& spv_tools_bin_dir, const std::string& spv_txt_file_path,
    const std::string& spv_file_path, bool should_print_cmd, std::string& error_msg)
{
    beStatus status = kBeStatusVulkanSpvAsmFailed;
    const std::string& opts = ConstructSpvAsmOptions(spv_txt_file_path, spv_file_path);
    if (!opts.empty())
    {
        std::string out_msg;
        status = InvokeSpvTool(BeVulkanSpirvTool::kAssembler, spv_tools_bin_dir, opts, should_print_cmd, out_msg, error_msg);

        // Check if the assembler has generated expected output file.
        if (status == kBeStatusSuccess && !KcUtils::FileNotEmpty(spv_file_path))
        {
            status = kBeStatusVulkanSpvAsmFailed;
        }
    }

    return status;
}

beKA::beStatus beProgramBuilderVulkan::PreprocessSource(const Config& config, const std::string& glslang_bin_dir, const std::string& input_file,
    bool is_hlsl, bool should_print_cmd, std::string& output, std::string& error_msg)
{
    beStatus status = kBeStatusVulkanPreprocessFailed;
    const std::string& opts = ConstructGlslangOptions(config, input_file, "", BePipelineStage::kCount, is_hlsl, true);
    assert(!opts.empty());
    if (!opts.empty())
    {
        status = InvokeGlslang(glslang_bin_dir, opts, should_print_cmd, output, error_msg);
    }

    return status;
}
