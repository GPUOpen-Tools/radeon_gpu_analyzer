//=================================================================
// Copyright 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifdef _WIN32

// C++.
#include <string>
#include <sstream>
#include <cassert>

// Infra.
#include "external/amdt_base_tools/Include/gtString.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osApplication.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_string_constants.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_dx12.h"

// CLI.
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"

// DXR.
#include "utils/dx12/backend/rg_dxr_state_desc_reader.h"
#include "utils/dx12/backend/rg_dxr_output_metadata.h"
#include "source/radeon_gpu_analyzer_cli/kc_cli_string_constants.h"

using namespace rga;

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// DX12.
static const wchar_t* kStrEnvVarNameAmdVirtualGpuId = L"AmdVirtualGpuId";
static const wchar_t* kDx12BackendDir               = L"utils/dx12";
static const wchar_t* kDx12BackendExe               = L"dx12_backend";
static const wchar_t* DX12_DXC_DIR                  = L"utils/dx12/dxc";
static const wchar_t* DX12_DXC_EXE                  = L"dxc";
static const wchar_t* kDx12OfflineBackend           = L"withDll";
static const wchar_t* kDx12OfflineKmtmuxer          = L"umdrepoint";
static const wchar_t* kDx12AmdxcBundledDriverDir    = L"utils/dx12/amdxc";
static const wchar_t* kDx12AmdxcBundledDriver       = L"amdxc64";
static const char*    kDx12BackendExeAscii          = "dx12_backend";
static const char*    kDx12OfflineBackendAscii      = "withdll";

// Error messages.
static const char* kStrErrorNoSupportedTarget1 = "Error: no supported target detected for '";
static const char* kStrErrorNoSupportedTarget2 = "', ";
static const char* kStrErrorNoSupportedTargetHintInstallDriver = "which is the default target. Please make sure that you have the latest AMD drivers installed.";
static const char* kStrErrorNoSupportedTargetHintRetrieveDx12Targets = "For the list of supported targets run: rga -s dx12 -l";
static const char* kStrErrorFailedToSetEnvVar = "Error: failed to set the environment variable for the DX12 backend.";
static const char* kStrErrorCannotRetrieveSupportedTargetList = "Error: cannot retrieve the list of targets supported by the driver. Consider adding --offline to the rga command to use the AMD driver (amdxc64.dll) that is bundled with the tool.";
static const char* kStrErrorFailedToInvokeDx12Backend = "Error: failed to invoke the DX12 backend.";
static const char* kStrErrorInvalidShaderModel = "Error: invalid shader model: ";
static const char* kStrErrorHlslToDxilCompilationFailed1 = "Error: DXC HLSL->DXIL compilation of ";
static const char* kStrErrorHlslToDxilCompilationFailed2 = " shader failed.";
static const char* kStrErrorDxcLaunchFailed = "failed to launch DXC.";

// Info messages.
static const char* kStrInfoFrontEndCompilationWithDxc1 = "Performing front-end compilation of ";
static const char* kStrInfoFrontEndCompilationWithDxc2 = " shader through DXC... ";
static const char* kStrInfoFrontEndCompilationWithDxcDxr = " HLSL file through DXC.";
static const char* kStrInfoFrontEndCompilationSuccess = "Front-end compilation success.";
static const char* kStrInfoDxcOutputPrologue = "*** Output from DXC - START ***";
static const char* kStrInfoDxcOutputEpilogue = "*** Output from DXC - END ***";
static const char* kStrInfoDxcUsingDxcFromUserPath = "Using DXC from user-provided path: ";
static const char* kStrInfoDxcReadingOptionsFromFile = "Info: reading additional DXC options from file: ";
static const char* kStrInfoDxcOptionsFileReadSuccess = "Info: DXC options file read successfully. Passing DXC the following options: ";

// Warning messages.
static const char* kStrWarningDxcPathNotFound1 = "Warning: could not detect DXC in path: ";
static const char* kStrWarningDxcPathNotFound2 = ". Falling back to using the DXC package that ships with RGA.";
static const char* kStrWarningDxcOptionsFileEmpty = "Warning: DXC options file empty: ";
static const char* kStrWarningDxcOptionsFileMissing = "Warning: DXC options file not found: ";

// Tokens.
const char* kStrDx12TokenBackendErrorToken = "Error";

// Suffixes for stage-specific output files.
static const std::array<std::string, BePipelineStage::kCount>
STR_DX12_STAGE_SUFFIX =
{
    "vert",
    "hull",
    "domain",
    "geom",
    "pixel",
    "comp"
};

// Other constants.
static const char* FILE_NAME_TOKEN_DXR = "*";
static const char* STR_DXIL_FILE_NAME = "dxil";
static const char* STR_DXIL_FILE_SUFFIX = "obj";

static void AddDebugLayerCommand(const Config &config, std::stringstream& cmd)
{
    if (config.dx12_debug_layer_enabled)
    {
        cmd << "--debug-layer " << std::endl;
    }
}

static std::string GetShaderModelPrefix(BePipelineStage stage)
{
    std::string ret;
    switch (stage)
    {
    case kVertex:
        ret = "vs";
        break;
    case kTessellationControl:
        ret = "hs";
        break;
    case kTessellationEvaluation:
        ret = "ds";
        break;
    case kGeometry:
        ret = "gs";
        break;
    case kFragment:
        ret = "ps";
        break;
    case kCompute:
        ret = "cs";
        break;
    case kCount:
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
    return ret;
}

// Filter out WithDll.exe's messages from the output unless in verbose mode (relevant to offline mode only).
static void FilterWithDllOutput(const Config& config, std::string& out_text)
{
    if (!config.print_process_cmd_line && config.dx12_offline_session)
    {
        size_t line_start = out_text.find(kDx12OfflineBackendAscii);
        while (line_start != std::string::npos)
        {
            size_t line_end = out_text.find('\n', line_start);
            auto   iter     = out_text.erase(line_start, line_end - line_start + 1);
            line_start      = out_text.find(kDx12OfflineBackendAscii);
        }
    }
}

static void FixIncludePath(const std::string& include_path, std::string& fixed_include_path)
{
    // DXC doesn't handle it when there is a '\' at the end of the path.
    // Remove that trailing '\' if it exists.
    fixed_include_path = include_path;
    if (!fixed_include_path.empty() && fixed_include_path.rfind('\\') == fixed_include_path.size() - 1)
    {
        fixed_include_path = fixed_include_path.substr(0, fixed_include_path.size() - 1);
    }
}

static bool InvokDxc(const Config& config, const std::string& shader_hlsl, const std::string& shader_model,
    const std::string& entry_point, const std::vector<std::string>& include_paths,
    const std::vector<std::string>& prepreocessor_defines,
    const std::string& output_filename, std::string& dxc_output, std::string& dxc_errors)
{
    bool ret = false;

    // Use DXC to compile shader model 5.1 or above.
    const char* kStrOptionTargetProfile = "-T";
    const char* kStrOptionOutputFile = "-Fo";
    const char* kStrOptionEntryPoint = "-E";
    const char* kStrOptionIncludePath = "-I";
    const char* kStrOptionPreprocessorDefines = "-D";
    const char* kStrOptionDxilDisassemblyOutputFile = "-Fc";

    // Construct the invocation command for DXC.
    std::stringstream cmd;

    // Target profile.
    cmd << kStrOptionTargetProfile << " " << shader_model << " ";

    if (!entry_point.empty())
    {
        // Entry point.
        cmd << kStrOptionEntryPoint << " " << entry_point << " ";
    }

    // Output file name.
    cmd << kStrOptionOutputFile << " \"" << output_filename << "\" ";

    // Include paths.
    for (const std::string& include_path : include_paths)
    {
        // DXC doesn't handle it when there is a '\' at the end of the path.
        std::string fixed_include_path;
        FixIncludePath(include_path, fixed_include_path);
        cmd << kStrOptionIncludePath << " \"" << fixed_include_path << "\" ";
    }

    // Preprocessor defines.
    for (const std::string& prepreocessor_define : prepreocessor_defines)
    {
        cmd << kStrOptionPreprocessorDefines << " " << prepreocessor_define << " ";
    }

    // DXIL disassembly.
    if (!config.cs_dxil_disassembly.empty())
    {
        cmd << kStrOptionDxilDisassemblyOutputFile << " \"" << config.cs_dxil_disassembly << "\" ";
    }
    else if (!config.vs_dxil_disassembly.empty() && shader_model.find("vs_") == 0)
    {
        cmd << kStrOptionDxilDisassemblyOutputFile << " \"" << config.vs_dxil_disassembly << "\" ";
    }
    else if (!config.hs_dxil_disassembly.empty() && shader_model.find("hs_") == 0)
    {
        cmd << kStrOptionDxilDisassemblyOutputFile << " \"" << config.hs_dxil_disassembly << "\" ";
    }
    else if (!config.ds_dxil_disassembly.empty() && shader_model.find("ds_") == 0)
    {
        cmd << kStrOptionDxilDisassemblyOutputFile << " \"" << config.ds_dxil_disassembly << "\" ";
    }
    else if (!config.gs_dxil_disassembly.empty() && shader_model.find("gs_") == 0)
    {
        cmd << kStrOptionDxilDisassemblyOutputFile << " \"" << config.gs_dxil_disassembly << "\" ";
    }
    else if (!config.ps_dxil_disassembly.empty() && shader_model.find("ps_") == 0)
    {
        cmd << kStrOptionDxilDisassemblyOutputFile << " \"" << config.ps_dxil_disassembly << "\" ";
    }

    // Additional options from the user if given.
    if (!config.dxc_opt.empty())
    {
        // Strip double quotes if necessary.
        std::string fixed_opt = config.dxc_opt;
        if (fixed_opt.size() > 3 && fixed_opt[0] == '"' && fixed_opt[fixed_opt.size() - 1] == '"')
        {
            fixed_opt = fixed_opt.substr(0, fixed_opt.size() - 1);
        }

        cmd << fixed_opt << " ";
    }
    else if (!config.dxc_opt_file.empty())
    {
        std::string dxc_opt_file_content;
        std::cout << kStrInfoDxcReadingOptionsFromFile << config.dxc_opt_file << std::endl;
        bool is_file_read = KcUtils::ReadTextFile(config.dxc_opt_file, dxc_opt_file_content, nullptr);
        assert(is_file_read);
        assert(!dxc_opt_file_content.empty());
        if (is_file_read)
        {
            if (!dxc_opt_file_content.empty())
            {
                cmd << dxc_opt_file_content << " ";
                std::cout << kStrInfoDxcOptionsFileReadSuccess << dxc_opt_file_content << std::endl;
            }
            else
            {
                std::cout << kStrWarningDxcOptionsFileEmpty << config.dxc_opt_file << std::endl;
            }
        }
        else
        {
            std::cout << kStrWarningDxcOptionsFileMissing << config.dxc_opt_file << std::endl;
        }
    }

    // Shader HLSL file.
    cmd << "\"" << shader_hlsl << "\"";

    long        exit_code = 0;
    osFilePath  dxc_exe;
    bool is_user_dxc_path_valid = false;
    if (!config.dxc_path.empty())
    {
        // Remove trailing "\" if present.
        std::string fixed_dxc_path = config.dxc_path;
        if (fixed_dxc_path.rfind('\\') == fixed_dxc_path.size() - 1)
        {
            fixed_dxc_path = fixed_dxc_path.substr(0, fixed_dxc_path.size() - 1);
        }

        osDirectory dxc_dir;
        std::cout << kStrInfoDxcUsingDxcFromUserPath << fixed_dxc_path << std::endl;
        gtString user_dxc_path;
        user_dxc_path << fixed_dxc_path.c_str();
        dxc_dir.setDirectoryFullPathFromString(user_dxc_path);
        dxc_exe.setFileDirectory(dxc_dir);
        if (dxc_exe.exists())
        {
            dxc_exe.setFileName(DX12_DXC_EXE);
            is_user_dxc_path_valid = true;
        }
        else
        {
            std::cout << kStrWarningDxcPathNotFound1 <<
                fixed_dxc_path << kStrWarningDxcPathNotFound2 << std::endl;
        }
    }

    if (!is_user_dxc_path_valid)
    {
        osGetCurrentApplicationPath(dxc_exe, false);
        dxc_exe.appendSubDirectory(DX12_DXC_DIR);
        dxc_exe.setFileName(DX12_DXC_EXE);
    }

    // Clear the error message buffer.
    dxc_output.clear();
    dxc_errors.clear();

    KcUtils::ProcessStatus status = KcUtils::LaunchProcess(dxc_exe.asString().asASCIICharArray(),
        cmd.str(),
        "",
        kProcessWaitInfinite,
        config.print_process_cmd_line,
        dxc_output,
        dxc_errors,
        exit_code);

    assert(status == KcUtils::ProcessStatus::kSuccess);
    ret = (status == KcUtils::ProcessStatus::kSuccess);

    return ret;
}

static bool CompileHlslWithDxcDxr(const Config& config, const std::string& hlsl_file, const std::string& shader_model, std::string& dxil_output_file)
{
    bool ret = false;
    const char* kDxrFileStageName = "dxr";
    bool shouldAbort = !KcUtils::ConstructOutFileName("", kDxrFileStageName,
        STR_DXIL_FILE_NAME, STR_DXIL_FILE_SUFFIX, dxil_output_file);
    assert(!shouldAbort);
    if (!shouldAbort)
    {
        // Notify the user.
        std::cout << kStrInfoFrontEndCompilationWithDxc1 << hlsl_file <<
            kStrInfoFrontEndCompilationWithDxcDxr << std::endl;

        // Perform the front-end compilation through DXC.
        std::string dxc_output;
        std::string dxc_errors;
        bool is_launch_successful = InvokDxc(config, hlsl_file, shader_model,
            "", config.include_path, config.defines, dxil_output_file, dxc_output, dxc_errors);
        assert(is_launch_successful);
        bool is_dxc_text_output_available = !dxc_output.empty() || !dxc_errors.empty();
        if (is_dxc_text_output_available)
        {
            // Inform user that output from DXC is coming.
            std::cout << std::endl << kStrInfoDxcOutputPrologue << std::endl << std::endl;
        }
        if (!dxc_output.empty())
        {
            // Print output.
            std::cout << dxc_output << std::endl;
        }
        if (!dxc_errors.empty())
        {
            // Print errors.
            std::cout << dxc_errors << std::endl;
        }
        if (is_dxc_text_output_available)
        {
            // Print DXC output epilogue.
            std::cout << kStrInfoDxcOutputEpilogue << std::endl;
        }

        if (!is_launch_successful)
        {
            // If no error messages printed by
            // front-end compiler, assume failure to launch.
            if (dxc_output.empty() && dxc_errors.empty())
            {
                std::cout << std::endl << kStrErrorDxcLaunchFailed << std::endl;
            }
        }
        else
        {
            bool is_dxil_file_valid = KcUtils::FileNotEmpty(dxil_output_file);
            if (is_dxil_file_valid)
            {
                std::cout << kStrInfoFrontEndCompilationSuccess << std::endl;
                ret = true;
            }
            else
            {
                std::cout << kStrErrorHlslToDxilCompilationFailed1 <<
                    hlsl_file << "." << std::endl;
                ret = false;
            }
        }

    }

    return ret;
}

static bool CompileHlslWithDxc(const Config& config, const std::string& hlsl_file, BePipelineStage stage, const std::string& shader_model,
    const std::string& entry_point, std::string& dxil_output_file)
{
    bool ret = false;
    bool should_abort = !KcUtils::ConstructOutFileName("", STR_DX12_STAGE_SUFFIX[stage],
        STR_DXIL_FILE_NAME, STR_DXIL_FILE_SUFFIX, dxil_output_file);
    assert(!should_abort);
    if (!should_abort)
    {
        // Notify the user.
        std::cout << kStrInfoFrontEndCompilationWithDxc1 << kStrDx12StageNames[stage] <<
            kStrInfoFrontEndCompilationWithDxc2 << std::endl;

        // Perform the front-end compilation through DXC.
        std::string dxc_output;
        std::string dxc_errors;
        bool is_launch_successful = InvokDxc(config, hlsl_file, shader_model,
            entry_point, config.include_path, config.defines, dxil_output_file, dxc_output, dxc_errors);
        assert(is_launch_successful);
        bool is_dxc_text_output_available = !dxc_output.empty() || !dxc_errors.empty();
        if (is_dxc_text_output_available)
        {
            // Inform user that output from DXC is coming.
            std::cout << std::endl << kStrInfoDxcOutputPrologue << std::endl << std::endl;
        }
        if (!dxc_output.empty())
        {
            // Print output.
            std::cout << dxc_output << std::endl;
        }
        if (!dxc_errors.empty())
        {
            // Print errors.
            std::cout << dxc_errors << std::endl;
        }
        if (is_dxc_text_output_available)
        {
            // Print DXC output epilogue.
            std::cout << kStrInfoDxcOutputEpilogue << std::endl;
        }

        if (!is_launch_successful)
        {
            // If no error messages printed by
            // front-end compiler, assume failure to launch.
            if (dxc_output.empty() && dxc_errors.empty())
            {
                std::cout << std::endl << kStrErrorDxcLaunchFailed << std::endl;
            }
        }
        else
        {
            bool isDxilFileValid = KcUtils::FileNotEmpty(dxil_output_file);
            if (isDxilFileValid)
            {
                std::cout << kStrInfoFrontEndCompilationSuccess << std::endl;
                ret = true;
            }
            else
            {
                std::cout << kStrErrorHlslToDxilCompilationFailed1 <<
                    kStrDx12StageNames[stage] << kStrErrorHlslToDxilCompilationFailed2 << std::endl;
                ret = false;
            }
        }
    }
    return ret;
}

// Sets output parameter to true if this shader model is a "legacy" shader model,
// which means that it is supported by D3DCompileFromFile through DXBC rather than DXIL.
// Namely, checks if the shader model is 5.1 or above.
// In case of a failure, for example, if the shader model format is invalid, false
// is return. Otherwise, true is returned.
static bool IsLegacyShaderModel(const std::string& shader_model, bool& is_legacy)
{
    const char* kStrErrorFailedParsingShaderModel = "Error: failed parsing shader model string.";
    bool ret = true;
    try
    {
        std::vector<std::string> model_components;
        BeUtils::SplitString(shader_model, '_', model_components);
        assert(model_components.size() > 2);
        if (model_components.size() > 2)
        {
            bool is_numeric_value = BeUtils::IsNumericValue(model_components[1]);
            assert(is_numeric_value);
            if (is_numeric_value)
            {
                int version_major = std::stoi(model_components[1]);
                if (version_major < 5)
                {
                    // If shader model is lower than 5, it's legacy.
                    is_legacy = true;
                }
                else if (version_major == 5)
                {
                    is_numeric_value = BeUtils::IsNumericValue(model_components[2]);
                    assert(is_numeric_value);
                    if (is_numeric_value)
                    {
                        int version_minor = std::stoi(model_components[2]);
                        if (version_minor == 0)
                        {
                            // If shader model is 5.0, it's legacy.
                            is_legacy = true;
                        }
                    }
                    else
                    {
                        // Invalid shader model.
                        ret = false;
                    }
                }
            }
            else
            {
                // Invalid shader model.
                ret = false;
            }
        }
    }
    catch (...)
    {
        std::cout << kStrErrorFailedParsingShaderModel << std::endl;
        ret = false;
    }
    return ret;
}

bool BeUtils::IsNumericValue(const std::string& str)
{
    return !str.empty() && std::find_if(str.begin(),
        str.end(), [](char c) { return !std::isdigit(c); }) == str.end();
}

static beStatus InvokeDx12Backend(const Config& config, const std::string& cmd_line_options, bool should_print_cmd,
    std::string& out_text, std::string& error_msg)
{
    beStatus ret = beStatus::kBeStatusSuccess;

    osFilePath dx12_backend_exe;
    osFilePath dx12_offline_exe;
    osFilePath dx12_kmtmuxer;
    osFilePath dx12_amdxc_driver;
    long exit_code = 0;

    // DX12 backend.
    osGetCurrentApplicationPath(dx12_backend_exe, false);
    dx12_backend_exe.appendSubDirectory(kDx12BackendDir);
    dx12_backend_exe.setFileName(kDx12BackendExe);

    // DX12 offline invocation.
    osGetCurrentApplicationPath(dx12_offline_exe, false);
    dx12_offline_exe.appendSubDirectory(kDx12BackendDir);
    dx12_offline_exe.setFileName(kDx12OfflineBackend);

    // Clear the error message buffer.
    error_msg.clear();

    // If we are in an offline session, update the command accordingly.
    std::string       cmd_line_updated;
    std::stringstream cmd_update_stream;

    if (config.dx12_offline_session)
    {
        // Kmtmuxer.
        osGetCurrentApplicationPath(dx12_kmtmuxer, false);
        dx12_kmtmuxer.appendSubDirectory(kDx12BackendDir);
        dx12_kmtmuxer.setFileName(kDx12OfflineKmtmuxer);
        dx12_kmtmuxer.setFileExtension(L"dll");

        // amdxc64 bundled driver.
        bool is_alternative_driver_path_found = false;
        if (!config.alternative_amdxc.empty())
        {
            // Use the user provided driver.
            const char *kDx12DriverName = "amdxc64.dll";
            is_alternative_driver_path_found = (config.alternative_amdxc.find(kDx12DriverName) != std::string::npos) &&
                KcUtils::FileNotEmpty(config.alternative_amdxc);
            if (!is_alternative_driver_path_found)
            {
                std::cout << "Warning: cannot find the provided alternative amdxc64.dll: " << config.alternative_amdxc << std::endl;
                std::cout << "Warning: falling back to using the bundled driver." << std::endl;
            }
            else
            {
                gtString amdxc_path_gtstr;
                amdxc_path_gtstr << config.alternative_amdxc.c_str();
                dx12_amdxc_driver.setFullPathFromString(amdxc_path_gtstr);
            }
        }

        if (!is_alternative_driver_path_found)
        {
            // Use the bundled driver.
            osGetCurrentApplicationPath(dx12_amdxc_driver, false);
            dx12_amdxc_driver.appendSubDirectory(kDx12AmdxcBundledDriverDir);
            dx12_amdxc_driver.setFileName(kDx12AmdxcBundledDriver);
            dx12_amdxc_driver.setFileExtension(L"dll");

            is_alternative_driver_path_found = dx12_amdxc_driver.exists();
            if (!is_alternative_driver_path_found)
            {
                std::cout << "Error: bundled DX12 driver (amdxc64.dll) is missing: " << dx12_amdxc_driver.asString().asASCIICharArray() << std::endl;
                ret = beStatus::kBeStatusDx12AlternativeDriverMissing;
            }
        }

        if (is_alternative_driver_path_found)
        {
            std::cout << "Info: in an offline session." << std::endl;

            // Use kmtmuxer.
            cmd_update_stream << " /d:\"" << dx12_kmtmuxer.asString().asASCIICharArray() << "\" ";

            // Set the environment variable.
            wchar_t* kKmtMuxerEnvVar = L"URSubDx12Path";
            BOOL     rc              = SetEnvironmentVariable(kKmtMuxerEnvVar, dx12_amdxc_driver.asString().asCharArray());
            std::cout << "Info: using amdxc64.dll from " << dx12_amdxc_driver.asString().asASCIICharArray() << std::endl;
            assert(rc == TRUE);

            // Append dx12_backend.exe to the command.
            cmd_update_stream << kDx12BackendExeAscii << " " << cmd_line_options << " --offline ";
        }
    }
    else
    {
        cmd_update_stream << cmd_line_options;
    }
    cmd_line_updated = cmd_update_stream.str().c_str();

    if (ret == beStatus::kBeStatusSuccess)
    {
        const osFilePath&      backend_exe = (config.dx12_offline_session ? dx12_offline_exe : dx12_backend_exe);
        std::stringstream      backend_exe_fixed;
        backend_exe_fixed << "\"" << backend_exe.asString().asASCIICharArray() << "\"";
        KcUtils::ProcessStatus status = KcUtils::LaunchProcess(backend_exe_fixed.str().c_str(), cmd_line_updated,
            "", kProcessWaitInfinite, should_print_cmd, out_text, error_msg, exit_code);
        assert(status == KcUtils::ProcessStatus::kSuccess);
        ret = (status == KcUtils::ProcessStatus::kSuccess ? kBeStatusSuccess : kBeStatusdx12BackendLaunchFailure);

        if (ret)
        {
            // Filter out WithDll.exe's messages from the output unless in verbose mode.
            FilterWithDllOutput(config, out_text);
        }
    }

    return ret;
}

static bool IsSupportedDevice(const std::string& device_name)
{
    const std::vector<std::string> kUnsupportedTargets = { "NAVI12LITE" };

    bool ret = true;
    for (const std::string& unsupported_target : kUnsupportedTargets)
    {
        if (device_name.find(unsupported_target) != std::string::npos)
        {
            ret = false;
            break;
        }
    }
    return ret;
}

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

beKA::beStatus BeProgramBuilderDx12::GetSupportGpus(const Config& config,
    std::vector<std::string>& gpus, std::map<std::string, int>& driver_ids)
{
    driver_ids.clear();
    std::string errors;

    // Retrieve the list of targets.
    std::string supported_gpus;
    BOOL rc = SetEnvironmentVariable(kStrEnvVarNameAmdVirtualGpuId, L"0");
    assert(rc == TRUE);

    beStatus ret = InvokeDx12Backend(config, "-l", config.print_process_cmd_line, supported_gpus, errors);
    assert(ret = beStatus::kBeStatusSuccess);
    assert(!supported_gpus.empty());
    if ((ret == beStatus::kBeStatusSuccess) && !supported_gpus.empty())
    {
        // Post-process the list.
        std::vector<std::string> split_gpu_names;
        BeUtils::SplitString(supported_gpus, '\n', split_gpu_names);
        assert(!split_gpu_names.empty());
        if (!split_gpu_names.empty())
        {
            // Name format is: <codename>:<gfxName>-><driver id>. We want both codename and gfxName,
            // then they would be passed to the code that compares them to known gpu names and filters as necessary.
            for (const std::string& driver_name : split_gpu_names)
            {
                if (driver_name.find("withdll") == std::string::npos && IsSupportedDevice(driver_name))
                {
                    // Break by ':'.
                    std::vector<std::string> split_names_colon;
                    BeUtils::SplitString(driver_name, ':', split_names_colon);
                    assert(!split_names_colon.empty());
                    if (!split_names_colon.empty())
                    {
                        std::transform(split_names_colon[0].begin(),
                            split_names_colon[0].end(), split_names_colon[0].begin(), ::tolower);

                        // Check if the name needs to be corrected to the "standard" name used by this tool.
                        auto corrected_name = std::find_if(kPalDeviceNameMapping.cbegin(), kPalDeviceNameMapping.cend(),
                            [&](const std::pair<std::string, std::string>& device) { return (device.first == split_names_colon[0]); });

                        if (corrected_name == kPalDeviceNameMapping.end())
                        {
                            gpus.push_back(split_names_colon[0]);
                        }
                        else
                        {
                            gpus.push_back(corrected_name->second);
                        }

                        assert(split_names_colon.size() > 1);
                        if (split_names_colon.size() > 1)
                        {
                            // Break by '->'.
                            std::vector<std::string> split_names_arrow;
                            BeUtils::SplitString(split_names_colon[1], '-', split_names_arrow);
                            assert(!split_names_arrow.empty());
                            if (!split_names_arrow.empty())
                            {
                                std::transform(split_names_arrow[0].begin(),
                                    split_names_arrow[0].end(), split_names_arrow[0].begin(), ::tolower);
                                gpus.push_back(split_names_arrow[0]);

                                // Get the id for this device.
                                std::vector<std::string> split_triangular_bracket;
                                BeUtils::SplitString(split_names_arrow[1], '>', split_triangular_bracket);
                                assert(split_triangular_bracket.size() > 1);
                                if (split_triangular_bracket.size() > 1)
                                {
                                    try
                                    {
                                        // Track the ID for both codename and gfx name.
                                        int id = std::stoi(split_triangular_bracket[1], nullptr);
                                        driver_ids[split_names_colon[0]] = id;
                                        driver_ids[split_names_arrow[0]] = id;

                                        // If we corrected the name, track the corrected codename as well.
                                        if (corrected_name != kPalDeviceNameMapping.end())
                                        {
                                            driver_ids[corrected_name->second] = id;
                                        }
                                    }
                                    catch (...)
                                    {
                                        const char* kStrErrorFailedToParseDeviceId = "Error: failed to parse device ID: ";
                                        std::cout << kStrErrorFailedToParseDeviceId <<
                                            split_triangular_bracket[1] << std::endl;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            assert(!gpus.empty());
            std::vector<GDT_GfxCardInfo> card_list;
            std::set<std::string> known_arch_names;
            // Get the list of known GPU architectures from DeviceInfo.
            bool is_all_cards_extracted = BeUtils::GetAllGraphicsCards(card_list, known_arch_names, true);
            assert(is_all_cards_extracted);
            if (is_all_cards_extracted)
            {
                for (auto iter = gpus.begin(); iter != gpus.end();)
                {
                    if (std::find(known_arch_names.begin(), known_arch_names.end(), (*iter)) == known_arch_names.end())
                    {
                        iter = gpus.erase(iter);
                    }
                    else
                    {
                        ++iter;
                    }
                }
            }
        }
    }
    else
    {
        if (!errors.empty())
        {
            std::cout << errors << std::endl;
        }
        std::cout << kStrErrorFailedToInvokeDx12Backend << std::endl;
    }

    return ret;
}

std::string GenerateShaderModel(const std::string &shader_model, const Config &config, BePipelineStage stage)
{
    std::string ret;
    if (!shader_model.empty())
    {
        ret = shader_model;
    }
    else
    {
        // Auto-generate the shader model.
        bool is_shader_model_generated = false;
        if (!config.all_model.empty())
        {
            std::stringstream model;
            std::string shader_model_prefix = GetShaderModelPrefix(stage);
            if (!shader_model_prefix.empty())
            {
                model << shader_model_prefix << "_" << config.all_model;
                ret = model.str();
                is_shader_model_generated = true;
            }
        }
    }
    return ret;
}

bool HandleHlslArgument(const Config &config, BePipelineStage stage,
    const char* stage_cmd_name, const std::string& hlsl_file,
    const std::string& shader_model, const std::string& entry_point,
    bool& is_legacy_shader_model, std::stringstream &cmd, std::string &dxil_compiled)
{
    bool ret = false;

    // Auto-generate the shader model if needed.
    std::string shader_model_generated = GenerateShaderModel(shader_model, config, stage);
    if (!shader_model_generated.empty())
    {
        bool is_shader_model_valid = IsLegacyShaderModel(shader_model_generated, is_legacy_shader_model);
        assert(is_shader_model_valid);
        if (is_shader_model_valid)
        {
            if (is_legacy_shader_model)
            {
                cmd << "--" << stage_cmd_name << " " << "\"" << hlsl_file << "\" ";
                ret = true;
            }
            else
            {
                // Compile the HLSL file through DXC.
                ret = CompileHlslWithDxc(config, hlsl_file,
                    stage, shader_model_generated, entry_point, dxil_compiled);
                if (ret)
                {
                    // Pass the compilation output to the backend as a binary.
                    cmd << "--" << stage_cmd_name << "-dxbc " << "\"" << dxil_compiled << "\" ";
                }
            }
        }
        else
        {
            std::cout << kStrErrorInvalidShaderModel << shader_model_generated << std::endl;
        }
    }
    else
    {
        std::cout << "Error: failed to auto-generate shader model for " << kStrDx12StageNames[stage] << " stage." << std::endl;
    }
    return ret;
}

bool BeProgramBuilderDx12::EnableNullBackendForDevice(const Config& config, const std::string& device_name)
{
    bool ret = false;
    try
    {
        if (code_name_to_driver_id_.empty())
        {
            std::vector<std::string> gpus;
            GetSupportGpus(config, gpus, code_name_to_driver_id_);
        }
        assert(!code_name_to_driver_id_.empty());
        if (!code_name_to_driver_id_.empty())
        {
            std::string target_device_lower = device_name;
            std::transform(target_device_lower.begin(), target_device_lower.end(),
                target_device_lower.begin(), ::tolower);
            auto iter = code_name_to_driver_id_.find(target_device_lower);
            if (iter != code_name_to_driver_id_.end())
            {
                // Set the environment variable.
                std::wstring value = std::to_wstring(iter->second);
                BOOL rc = SetEnvironmentVariable(kStrEnvVarNameAmdVirtualGpuId, value.c_str());
                assert(rc == TRUE);
                ret = (rc == TRUE);
                if (!ret)
                {
                    std::cout << kStrErrorFailedToSetEnvVar << std::endl;
                }
            }
            else
            {
                std::cout << kStrErrorNoSupportedTarget1 << device_name << kStrErrorNoSupportedTarget2;
                if (config.asics.empty())
                {
                    // If this is the default device, the user needs to install the latest AMD drivers.
                    std::cout << kStrErrorNoSupportedTargetHintInstallDriver;
                }

                std::cout << std::endl  << kStrErrorNoSupportedTargetHintRetrieveDx12Targets << std::endl;
            }
        }
        else
        {
            std::cout << kStrErrorCannotRetrieveSupportedTargetList << std::endl;
        }
    }
    catch (...)
    {
        ret = false;
    }

    return ret;
}

beKA::beStatus BeProgramBuilderDx12::Compile(const Config& config,
 const std::string& target_device, std::string& out_text, std::string& error_msg, BeVkPipelineFiles& generated_isa_files,
    BeVkPipelineFiles& generated_amdil_files, BeVkPipelineFiles& generated_stats_files, std::string& generated_binary_files)
{
    beKA::beStatus ret = beStatus::kBeStatusInvalid;
    bool rc = EnableNullBackendForDevice(config, target_device);
    assert(rc);
    if (rc)
    {
        // For each stage, check if the target profile is of a legacy shader model (5.0 or below).
        // If this is the case, we will pass the backend the required parameters to perform the
        // compilation through the runtime into DXBC. Otherwise, we will compile first through DXC
        // into DXIL, and then use the generated binary as a pre-compiled input for the backend.
        bool is_legacy_shader_model_vs = false;
        bool is_legacy_shader_model_hs = false;
        bool is_legacy_shader_model_ds = false;
        bool is_legacy_shader_model_gs = false;
        bool is_legacy_shader_model_ps = false;
        bool is_legacy_shader_model_cs = false;

        // Names of DXIL files generated by this stage in
        // case that the shader model is not a legacy one.
        std::string dxil_compiled_vs;
        std::string dxil_compiled_hs;
        std::string dxil_compiled_ds;
        std::string dxil_compiled_gs;
        std::string dxil_compiled_ps;
        std::string dxil_compiled_cs;

        // Build the command line invocation for the backend.
        std::stringstream cmd;

        // True if we should abort the compilation process.
        bool should_abort = false;

        // Generate a lower-case version of the target name.
        std::string target_device_lower = target_device;
        std::transform(target_device_lower.begin(), target_device_lower.end(),
            target_device_lower.begin(), ::tolower);

        // Input files - HLSL.
        if (!should_abort && !config.vs_hlsl.empty())
        {
            should_abort = !HandleHlslArgument(config, BePipelineStage::kVertex, "vert", config.vs_hlsl,
                config.vs_model, config.vs_entry_point, is_legacy_shader_model_vs, cmd, dxil_compiled_vs);
        }
        if (!should_abort && !config.hs_hlsl.empty())
        {
            should_abort = !HandleHlslArgument(config, BePipelineStage::kTessellationControl, "hull", config.hs_hlsl,
                config.hs_model, config.hs_entry_point, is_legacy_shader_model_hs, cmd, dxil_compiled_hs);
        }
        if (!should_abort && !config.ds_hlsl.empty())
        {
            should_abort = !HandleHlslArgument(config, BePipelineStage::kTessellationEvaluation, "domain", config.ds_hlsl,
                config.ds_model, config.ds_entry_point, is_legacy_shader_model_ds, cmd, dxil_compiled_ds);
        }
        if (!should_abort && !config.gs_hlsl.empty())
        {
            should_abort = !HandleHlslArgument(config, BePipelineStage::kGeometry, "geom", config.gs_hlsl,
                config.gs_model, config.gs_entry_point, is_legacy_shader_model_gs, cmd, dxil_compiled_gs);
        }
        if (!should_abort && !config.ps_hlsl.empty())
        {
            should_abort = !HandleHlslArgument(config, BePipelineStage::kFragment, "pixel", config.ps_hlsl,
                config.ps_model, config.ps_entry_point, is_legacy_shader_model_ps, cmd, dxil_compiled_ps);
        }
        if (!should_abort && !config.cs_hlsl.empty())
        {
            should_abort = !HandleHlslArgument(config, BePipelineStage::kCompute, "comp", config.cs_hlsl,
                config.cs_model, config.cs_entry_point, is_legacy_shader_model_cs, cmd, dxil_compiled_cs);

            // Special case: if we switched HLSL input with binary input and RS is in HLSL file and
            // needs to be compiled from a macro, and if an HLSL file has not been given by the user since
            // this is a compute shader which only requires a single HLSL path, pass the HLSL path option
            // so that the backend knows where to read the macro from.
            if (!dxil_compiled_cs.empty() && !config.rs_macro.empty() && config.rs_hlsl.empty())
            {
                cmd << " --rs-hlsl " << "\"" << config.cs_hlsl << "\" ";
            }
        }

        if (!should_abort)
        {
            // Input files - DXBC.
            if (!config.vs_dxbc.empty())
            {
                cmd << "--vert-dxbc " << "\"" << config.vs_dxbc << "\" ";
            }
            if (!config.hs_dxbc.empty())
            {
                cmd << "--hull-dxbc " << "\"" << config.hs_dxbc << "\" ";
            }
            if (!config.ds_dxbc.empty())
            {
                cmd << "--domain-dxbc " << "\"" << config.ds_dxbc << "\" ";
            }
            if (!config.gs_dxbc.empty())
            {
                cmd << "--geom-dxbc " << "\"" << config.gs_dxbc << "\" ";
            }
            if (!config.ps_dxbc.empty())
            {
                cmd << "--pixel-dxbc " << "\"" << config.ps_dxbc << "\" ";
            }
            if (!config.cs_dxbc.empty())
            {
                cmd << "--comp-dxbc " << "\"" << config.cs_dxbc << "\" ";
            }

            // Entry point.
            if (!config.vs_entry_point.empty())
            {
                cmd << "--vert-entry " << config.vs_entry_point << " ";
            }
            if (!config.hs_entry_point.empty())
            {
                cmd << "--hull-entry " << config.hs_entry_point << " ";
            }
            if (!config.ds_entry_point.empty())
            {
                cmd << "--domain-entry " << config.ds_entry_point << " ";
            }
            if (!config.gs_entry_point.empty())
            {
                cmd << "--geom-entry " << config.gs_entry_point << " ";
            }
            if (!config.ps_entry_point.empty())
            {
                cmd << "--pixel-entry " << config.ps_entry_point << " ";
            }
            if (!config.cs_entry_point.empty())
            {
                cmd << "--comp-entry " << config.cs_entry_point << " ";
            }

            // Shader model.
            std::string vs_model_generated = !config.vs_hlsl.empty() ? GenerateShaderModel(config.vs_model, config, BePipelineStage::kVertex) : "";
            std::string hs_model_generated = !config.hs_hlsl.empty() ? GenerateShaderModel(config.hs_model, config, BePipelineStage::kTessellationControl) : "";
            std::string ds_model_generated = !config.ds_hlsl.empty() ? GenerateShaderModel(config.ds_model, config, BePipelineStage::kTessellationEvaluation) : "";
            std::string gs_model_generated = !config.gs_hlsl.empty() ? GenerateShaderModel(config.gs_model, config, BePipelineStage::kGeometry) : "";
            std::string ps_model_generated = !config.ps_hlsl.empty() ? GenerateShaderModel(config.ps_model, config, BePipelineStage::kFragment) : "";
            std::string cs_model_generated = !config.cs_hlsl.empty() ? GenerateShaderModel(config.cs_model, config, BePipelineStage::kCompute) : "";

            if (!vs_model_generated.empty())
            {
                cmd << "--vert-target " << vs_model_generated << " ";
            }
            if (!hs_model_generated.empty())
            {
                cmd << "--hull-target " << hs_model_generated << " ";
            }
            if (!ds_model_generated.empty())
            {
                cmd << "--domain-target " << ds_model_generated << " ";
            }
            if (!gs_model_generated.empty())
            {
                cmd << "--geom-target " << gs_model_generated << " ";
            }
            if (!ps_model_generated.empty())
            {
                cmd << "--pixel-target " << ps_model_generated << " ";
            }
            if (!cs_model_generated.empty())
            {
                cmd << "--comp-target " << cs_model_generated << " ";
            }

            // Root signature.
            if (!config.rs_bin.empty())
            {
                cmd << "--rs-bin " << "\"" << config.rs_bin << "\" ";
            }
            if (!config.rs_hlsl.empty())
            {
                cmd << "--rs-hlsl " << "\"" << config.rs_hlsl << "\" ";
            }
            else if (!config.all_hlsl.empty())
            {
                cmd << "--rs-hlsl " << "\"" << config.all_hlsl << "\" ";
            }
            if (!config.rs_macro.empty())
            {
                cmd << "--rs-macro " << "\"" << config.rs_macro << "\" ";
            }
            if (!config.rs_macro_version.empty())
            {
                cmd << "--rs-macro-version " << "\"" << config.rs_macro_version << "\" ";
            }

            // Include.
            for (const std::string& include_path : config.include_path)
            {
                std::string fixed_include_path;
                FixIncludePath(include_path, fixed_include_path);
                cmd << "--include " << "\"" << fixed_include_path << "\" ";
            }

            // Preprocessor defines.
            for (const std::string& preprocessor_define : config.defines)
            {
                cmd << "--define " << preprocessor_define << " ";
            }

            // Package the ISA and statistics file names in separate containers for pre-processing.
            std::string input_files[BePipelineStage::kCount] = { config.vs_hlsl, config.hs_hlsl,
                config.ds_hlsl, config.gs_hlsl, config.ps_hlsl, config.cs_hlsl };

            // If DXBC is used as an input instead of HLSL, replace the input file.
            if (input_files[BePipelineStage::kVertex].empty() && !config.vs_dxbc.empty())
            {
                input_files[BePipelineStage::kVertex] = config.vs_dxbc;
            }
            if (input_files[BePipelineStage::kTessellationControl].empty() && !config.hs_dxbc.empty())
            {
                input_files[BePipelineStage::kTessellationControl] = config.hs_dxbc;
            }
            if (input_files[BePipelineStage::kTessellationEvaluation].empty() && !config.ds_dxbc.empty())
            {
                input_files[BePipelineStage::kTessellationEvaluation] = config.ds_dxbc;
            }
            if (input_files[BePipelineStage::kGeometry].empty() && !config.gs_dxbc.empty())
            {
                input_files[BePipelineStage::kGeometry] = config.gs_dxbc;
            }
            if (input_files[BePipelineStage::kFragment].empty() && !config.ps_dxbc.empty())
            {
                input_files[BePipelineStage::kFragment] = config.ps_dxbc;
            }
            if (input_files[BePipelineStage::kCompute].empty() && !config.cs_dxbc.empty())
            {
                input_files[BePipelineStage::kCompute] = config.cs_dxbc;
            }
            for (int stage = 0; stage < BePipelineStage::kCount; stage++)
            {
                if (!input_files[stage].empty())
                {
                    // ISA files.
                    if (!config.isa_file.empty())
                    {
                        bool isFileNameConstructed = KcUtils::ConstructOutFileName(config.isa_file,
                            STR_DX12_STAGE_SUFFIX[stage], target_device_lower, "isa", generated_isa_files[stage]);
                        assert(isFileNameConstructed);
                        if (isFileNameConstructed && !generated_isa_files[stage].empty())
                        {
                            cmd << " --" << STR_DX12_STAGE_SUFFIX[stage] << "-isa " << "\"" << generated_isa_files[stage] << "\" ";

                            // Delete that file if it already exists.
                            if (BeUtils::IsFilePresent(generated_isa_files[stage]))
                            {
                                BeUtils::DeleteFileFromDisk(generated_isa_files[stage]);
                            }
                        }
                    }

                    // AMDIL files.
                    if (!config.il_file.empty())
                    {
                        bool isFileNameConstructed = KcUtils::ConstructOutFileName(config.il_file,
                            STR_DX12_STAGE_SUFFIX[stage], target_device_lower, "amdil", generated_amdil_files[stage]);
                        assert(isFileNameConstructed);
                        if (isFileNameConstructed && !generated_amdil_files[stage].empty())
                        {
                            cmd << " --" << STR_DX12_STAGE_SUFFIX[stage] << "-amdil " << "\"" << generated_amdil_files[stage] << "\" ";

                            // Delete that file if it already exists.
                            if (BeUtils::IsFilePresent(generated_amdil_files[stage]))
                            {
                                BeUtils::DeleteFileFromDisk(generated_amdil_files[stage]);
                            }
                        }
                    }

                    // Statistics files.
                    if (!config.analysis_file.empty())
                    {
                        bool is_filename_constructed = KcUtils::ConstructOutFileName(config.analysis_file,
                            STR_DX12_STAGE_SUFFIX[stage], target_device_lower,
                            kStrDefaultExtensionStats, generated_stats_files[stage]);
                        assert(is_filename_constructed);
                        if (is_filename_constructed && !generated_stats_files[stage].empty())
                        {
                            cmd << " --" << STR_DX12_STAGE_SUFFIX[stage] << "-stats " << "\"" << generated_stats_files[stage] << "\" ";
                        }

                        // Delete that file if it already exists.
                        if (BeUtils::IsFilePresent(generated_isa_files[stage]))
                        {
                            BeUtils::DeleteFileFromDisk(generated_isa_files[stage]);
                        }
                    }
                }
            }

            // For older shader models, we should retrieve the DXBC disassembly from the runtime compiler.
            // For the newer shader models, this has already been done offline through DXC.
            bool is_legacy_shader_model = false;
            if (!config.vs_hlsl.empty() && !config.vs_dxil_disassembly.empty() &&
                IsLegacyShaderModel(vs_model_generated, is_legacy_shader_model) &&
                is_legacy_shader_model)
            {
                cmd << "--vert-dxbc-dis \"" << config.vs_dxil_disassembly << "\" ";
            }
            if (!config.hs_hlsl.empty() && !config.hs_dxil_disassembly.empty() &&
                IsLegacyShaderModel(hs_model_generated, is_legacy_shader_model) &&
                is_legacy_shader_model)
            {
                cmd << "--hull-dxbc-dis \"" << config.hs_dxil_disassembly << "\" ";
            }
            if (!config.ds_hlsl.empty() && !config.ds_dxil_disassembly.empty() &&
                IsLegacyShaderModel(ds_model_generated, is_legacy_shader_model) &&
                is_legacy_shader_model)
            {
                cmd << "--domain-dxbc-dis \"" << config.ds_dxil_disassembly << "\" ";
            }
            if (!config.gs_hlsl.empty() && !config.gs_dxil_disassembly.empty() &&
                IsLegacyShaderModel(gs_model_generated, is_legacy_shader_model) &&
                is_legacy_shader_model)
            {
                cmd << "--geom-dxbc-dis \"" << config.gs_dxil_disassembly << "\" ";
            }
            if (!config.ps_hlsl.empty() && !config.ps_dxil_disassembly.empty() &&
                IsLegacyShaderModel(ps_model_generated, is_legacy_shader_model) &&
                is_legacy_shader_model)
            {
                cmd << "--pixel-dxbc-dis \"" << config.ps_dxil_disassembly << "\" ";
            }
            if (!config.cs_hlsl.empty() && !config.cs_dxil_disassembly.empty() &&
                IsLegacyShaderModel(cs_model_generated, is_legacy_shader_model) &&
                is_legacy_shader_model)
            {
                cmd << "--comp-dxbc-dis \"" << config.cs_dxil_disassembly << "\" ";
            }

            // Pipeline binary.
            if (!config.binary_output_file.empty())
            {
                std::string binary_output_name_per_device;
                bool is_filename_constructed = KcUtils::ConstructOutFileName(config.binary_output_file,
                    "", target_device_lower, "bin", binary_output_name_per_device);
                assert(is_filename_constructed);
                if (is_filename_constructed && !binary_output_name_per_device.empty())
                {
                    cmd << "-b \"" << binary_output_name_per_device << "\" ";

                    // Track the binary file.
                    generated_binary_files = binary_output_name_per_device;
                }
            }

            // Graphics pipeline state.
            if (!config.pso_dx12.empty())
            {
                cmd << "--pso \"" << config.pso_dx12 << "\" ";
            }

            // D3D12 debug layer.
            AddDebugLayerCommand(config, cmd);

            // Invoke the backend to perform the actual build.
            ret = InvokeDx12Backend(config, cmd.str().c_str(), config.print_process_cmd_line, out_text, error_msg);
            assert(ret == kBeStatusSuccess);
            if (ret == kBeStatusSuccess)
            {
                bool is_success = out_text.find(kStrDx12TokenBackendErrorToken) == std::string::npos &&
                    error_msg.find(kStrDx12TokenBackendErrorToken) == std::string::npos;
                if (!is_success)
                {
                    ret = kBeStatusdx12CompileFailure;
                }
            }
            else
            {
                if (!error_msg.empty())
                {
                    std::cout << error_msg << std::endl;
                }
                std::cout << kStrErrorFailedToInvokeDx12Backend << std::endl;
            }

            // Cleanup: delete temporary files (DXIL files that were generated by DXC).
            if (!config.should_retain_temp_files)
            {
                if (KcUtils::FileNotEmpty(dxil_compiled_vs))
                {
                    KcUtils::DeleteFile(dxil_compiled_vs);
                }
                if (KcUtils::FileNotEmpty(dxil_compiled_hs))
                {
                    KcUtils::DeleteFile(dxil_compiled_hs);
                }
                if (KcUtils::FileNotEmpty(dxil_compiled_ds))
                {
                    KcUtils::DeleteFile(dxil_compiled_ds);
                }
                if (KcUtils::FileNotEmpty(dxil_compiled_gs))
                {
                    KcUtils::DeleteFile(dxil_compiled_gs);
                }
                if (KcUtils::FileNotEmpty(dxil_compiled_ps))
                {
                    KcUtils::DeleteFile(dxil_compiled_ps);
                }
                if (KcUtils::FileNotEmpty(dxil_compiled_cs))
                {
                    KcUtils::DeleteFile(dxil_compiled_cs);
                }
            }
        }
    }

    return ret;
}

beKA::beStatus BeProgramBuilderDx12::CompileDXRPipeline(const Config&                      config,
                                                        const std::string&                 target_device,
    std::string& out_text, std::vector<RgDxrPipelineResults>& output_mapping, std::string& error_msg)
{
    beKA::beStatus ret = beStatus::kBeStatusInvalid;
    bool rc = EnableNullBackendForDevice(config, target_device);
    assert(rc);
    if (rc)
    {
        bool should_abort = false;

        // Front-end compilation (if necessary).
        RgDxrStateDesc state_desc;
        if (config.dxr_hlsl.empty())
        {
            bool is_state_desc_read = RgDxrStateDescReader::ReadDxrStateDesc(config.dxr_state_desc, state_desc, error_msg);
            assert(is_state_desc_read);
            should_abort = !is_state_desc_read;
            if (!error_msg.empty())
            {
                std::cerr << error_msg << std::endl;
                error_msg.clear();
            }
        }
        else
        {
            // If an HLSL input was given, we will only use the HLSL as an input, without
            // reading the state data.
            std::shared_ptr<RgDxrDxilLibrary> dxil_lib = std::make_shared<RgDxrDxilLibrary>();
            dxil_lib->input_type = DxrSourceType::kHlsl;
            dxil_lib->full_path = config.dxr_hlsl;
            state_desc.input_files.push_back(dxil_lib);
        }

        // Track the compiled HLSL files.
        std::vector<std::string> temp_dxil_files;

        // Create a text file with a HLSL->DXIL mapping in case that
        // HLSL front-end compilation was performed.
        std::string hlsl_mapping_file;
        std::stringstream hlsl_mapping_content;

        // Compile HLSL files if needed.
        for (const auto& dxil_file : state_desc.input_files)
        {
            assert(dxil_file != nullptr);
            if (dxil_file != nullptr)
            {
                if (dxil_file->input_type == DxrSourceType::kHlsl)
                {
                    std::string curr_dxil_output_filename;
                    rc = CompileHlslWithDxcDxr(config, dxil_file->full_path, config.dxr_shader_model, curr_dxil_output_filename);
                    assert(rc);
                    assert(!curr_dxil_output_filename.empty());
                    if (rc && !curr_dxil_output_filename.empty())
                    {
                        temp_dxil_files.push_back(curr_dxil_output_filename);
                        hlsl_mapping_content << dxil_file->full_path << "$" << curr_dxil_output_filename << std::endl;
                    }
                }
            }
        }

        if (!temp_dxil_files.empty())
        {
            hlsl_mapping_file = KcUtils::ConstructTempFileName("rga-hlsl-dxr-mapping", kStrDefaultExtensionText);
            bool is_hlsl_mappinig_file_created =  KcUtils::WriteTextFile(hlsl_mapping_file, hlsl_mapping_content.str(), nullptr);
            assert(is_hlsl_mappinig_file_created);
            if (!is_hlsl_mappinig_file_created)
            {
                std::cout << "Error: failed to created HLSL mapping file." << std::endl;
                should_abort = true;
            }
        }

        should_abort = !rc;
        if (!should_abort)
        {
            std::stringstream cmd;
            if (config.dxr_hlsl.empty())
            {
                // State description file.
                cmd << "--state-desc " << "\"" << config.dxr_state_desc << "\"";
            }
            else
            {
                // HLSL input file.
                cmd << "--hlsl " << "\"" << config.dxr_hlsl << "\"";
            }

            // Mode.
            cmd << " --mode " << config.dxr_mode << " ";

            // Metadata output file, generate a temporary file for that.
            std::string metadata_filename = KcUtils::ConstructTempFileName("rga-dxr-output", kStrDefaultExtensionText);
            cmd << "--output-metadata " << "\"" << metadata_filename << "\"" << " ";

            for (const std::string& currExport : config.dxr_exports)
            {
                cmd << "--export " << currExport << " ";

                // In "all" mode, we need to generate the results for all generated pipelines, therefore we need
                // to generate the file names with a special token '*' which would be replaced by the backend
                // with the relevant index of that pipeline.
                bool is_all_mode = (config.dxr_exports.size() == 1 && config.dxr_exports[0].compare("all") == 0);
                bool is_pipeline_mode = (KcUtils::ToLower(config.dxr_mode).compare("pipeline") == 0);

                // ISA.
                if (!config.isa_file.empty())
                {
                    bool is_dir_output = KcUtils::IsDirectory(config.isa_file);
                    std::string patched_export_name = is_all_mode ? FILE_NAME_TOKEN_DXR : currExport;
                    if (!is_all_mode && is_pipeline_mode)
                    {
                        patched_export_name.append("_");
                        patched_export_name.append(FILE_NAME_TOKEN_DXR);
                    }

                    std::string generated_isa_filename;
                    bool is_filename_constructed = KcUtils::ConstructOutFileName(config.isa_file,
                        patched_export_name, target_device, "isa", generated_isa_filename, (is_pipeline_mode || !is_dir_output));
                    assert(is_filename_constructed);
                    if (is_filename_constructed && !generated_isa_filename.empty())
                    {
                        cmd << "--dxr-isa " << "\"" << generated_isa_filename << "\" ";

                        // Delete that file if it already exists.
                        if (BeUtils::IsFilePresent(generated_isa_filename))
                        {
                            BeUtils::DeleteFileFromDisk(generated_isa_filename);
                        }
                    }
                }

                // Add the required output files to the command.
                std::string target_device_lower = target_device;
                std::transform(target_device_lower.begin(), target_device_lower.end(),
                    target_device_lower.begin(), ::tolower);

                // Statistics files.
                if (!config.analysis_file.empty())
                {
                    bool is_dir_output = KcUtils::IsDirectory(config.analysis_file);
                    std::string patched_export_name = is_all_mode ? FILE_NAME_TOKEN_DXR : currExport;
                    if (!is_all_mode && is_pipeline_mode)
                    {
                        patched_export_name.append("_");
                        patched_export_name.append(FILE_NAME_TOKEN_DXR);
                    }

                    std::string generated_stats_filename;
                    bool is_filename_constructed = KcUtils::ConstructOutFileName(config.analysis_file,
                        patched_export_name, target_device_lower,
                        kStrDefaultExtensionStats, generated_stats_filename, (is_pipeline_mode || !is_dir_output));
                    assert(is_filename_constructed);
                    if (is_filename_constructed && !generated_stats_filename.empty())
                    {
                        cmd << "--dxr-stats " << "\"" << generated_stats_filename << "\" ";

                        // Delete that file if it already exists.
                        if (BeUtils::IsFilePresent(generated_stats_filename))
                        {
                            BeUtils::DeleteFileFromDisk(generated_stats_filename);
                        }
                    }
                }

                // Binary files.
                if (!config.binary_output_file.empty())
                {
                    std::string patched_export_name = is_all_mode ? FILE_NAME_TOKEN_DXR : currExport;
                    if (!is_all_mode && is_pipeline_mode)
                    {
                        patched_export_name.append("_");
                        patched_export_name.append(FILE_NAME_TOKEN_DXR);
                    }

                    std::string generated_binary_file;
                    bool is_filename_constructed = KcUtils::ConstructOutFileName(config.binary_output_file,
                        patched_export_name, target_device_lower,
                        "bin", generated_binary_file);
                    assert(is_filename_constructed);
                    if (is_filename_constructed && !generated_binary_file.empty())
                    {
                        cmd << "--dxr-bin " << "\"" << generated_binary_file << "\" ";
                    }

                    // Delete that file if it already exists.
                    if (BeUtils::IsFilePresent(generated_binary_file))
                    {
                        BeUtils::DeleteFileFromDisk(generated_binary_file);
                    }
                }
            }

            // HLSL->DXIL mapping.
            if (!hlsl_mapping_file.empty())
            {
                cmd << "--dxr-hlsl-mapping " << "\"" << hlsl_mapping_file << "\" ";
            }

            // D3D12 debug layer.
            AddDebugLayerCommand(config, cmd);

            // Invoke the backend to perform the actual build.
            ret = InvokeDx12Backend(config, cmd.str().c_str(), config.print_process_cmd_line, out_text, error_msg);
            assert(ret == kBeStatusSuccess);
            if (ret == kBeStatusSuccess)
            {
                bool is_success = out_text.find(kStrDx12TokenBackendErrorToken) == std::string::npos &&
                    error_msg.find(kStrDx12TokenBackendErrorToken) == std::string::npos;

                if (is_success)
                {
                    if (KcUtils::FileNotEmpty(metadata_filename))
                    {
                        is_success = RgDxrOutputMetadata::ReadOutputMetadata(metadata_filename, output_mapping, error_msg);
                        assert(is_success);
                        if (!is_success)
                        {
                            ret = kBeStatusdx12OutputMetadataMissing;
                        }

                        if (!config.should_retain_temp_files)
                        {
                            // Delete the temporary file.
                            KcUtils::DeleteFileW(metadata_filename);
                        }
                    }
                }
                else
                {
                    ret = kBeStatusdx12CompileFailure;
                }
            }
            else
            {
                if (!error_msg.empty())
                {
                    std::cout << error_msg << std::endl;
                }
                std::cout << kStrErrorFailedToInvokeDx12Backend << std::endl;
            }
        }

        // Clean up temporary files.
        if (!config.should_retain_temp_files)
        {
            KcUtils::DeleteFileW(hlsl_mapping_file);
            for (const std::string& tempDxilFile : temp_dxil_files)
            {
                KcUtils::DeleteFileW(tempDxilFile);
            }
        }
    }
    return ret;
}

#endif
