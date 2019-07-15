//=================================================================
// Copyright 2019 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifdef _WIN32

// C++.
#include <string>
#include <sstream>
#include <cassert>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osApplication.h>

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beStringConstants.h>
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderDX12.h>

// CLI.
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

static const char* STR_ERROR_NO_SUPPORTED_TARGET_A = "Error: no supported target detected for '";
static const char* STR_ERROR_NO_SUPPORTED_TARGET_B = "'";
static const char* STR_ERROR_FAILED_TO_SET_ENV_VAR = "Error: failed to set the environment variable for the DX12 backend.";
static const char* STR_ERROR_CANNOT_RETRIEVE_SUPPORTED_TARGET_LIST = "Error: cannot retrieve the list of targets supported by the driver.";
static const char* STR_ERROR_FAILED_TO_INVOKE_DX12_BACKEND = "Error: failed to invoke the DX12 backend.";
static const char* STR_ERROR_INVALID_SHADER_MODEL = "Error: invalid shader model: ";
static const char* STR_ERROR_HLSL_TO_DXIL_COMPILATION_FAILED_A = "Error: DXC HLSL->DXIL compilation of ";
static const char* STR_ERROR_HLSL_TO_DXIL_COMPILATION_FAILED_B = " shader failed.";
static const char* STR_ERROR_DXC_LAUNCH_FAILED = "failed to launch DXC.";
static const char* STR_INFO_FRONT_END_COMPILATION_WITH_DXC = "Performing front-end compilation through DXC... ";
static const char* STR_INFO_FRONT_END_COMPILATION_SUCCESS = "front-end compilation success.";
static const char* STR_INFO_DXC_OUTPUT_PROLOGUE = "*** Output from DXC - START ***";
static const char* STR_INFO_DXC_OUTPUT_EPILOGUE = "*** Output from DXC - END ***";

// Suffixes for stage-specific output files.
static const std::array<std::string, bePipelineStage::Count>
STR_DX12_STAGE_SUFFIX =
{
    "vert",
    "hull",
    "domain",
    "geom",
    "pixel",
    "comp"
};

static void FixIncludePath(const std::string& includePath, std::string& fixedIncludePath)
{
    // DXC doesn't handle it when there is a '\' at the end of the path.
    // Remove that trailing '\' if it exists.
    fixedIncludePath = includePath;
    if (!fixedIncludePath.empty() && fixedIncludePath.rfind('\\') == fixedIncludePath.size() - 1)
    {
        fixedIncludePath = fixedIncludePath.substr(0, fixedIncludePath.size() - 1);
    }
}

static bool InvokDxc(const Config& config, const std::string& shaderHlsl, const std::string& shaderModel,
    const std::string& entryPoint, const std::vector<std::string>& includePaths,
    const std::vector<std::string>& prepreocessorDefine,
    const std::string& outputFileName, std::string& dxcOutput, std::string& dxcErrors)
{
    bool ret = false;

    // Use FXC to compile shader model 5.1 or above.
    const char* STR_DXC_OPTION_TARGET_PROFILE = "-T";
    const char* STR_DXC_OPTION_OUTPUT_FILE = "-Fo";
    const char* STR_DXC_OPTION_ENTRY_POINT = "-E";
    const char* STR_DXC_OPTION_INCLUDE_PATH = "-I";
    const char* STR_DXC_OPTION_PREPROCESSOR_DEFINES = "-D";
    const char* STR_DXC_OPTION_DXIL_DISASSEMBLY_OUTPUT_FILE = "-Fc";

    // Construct the invocation command for DXC.
    std::stringstream cmd;

    // Target profile.
    cmd << STR_DXC_OPTION_TARGET_PROFILE << " " << shaderModel << " ";

    // Entry point.
    cmd << STR_DXC_OPTION_ENTRY_POINT << " " << entryPoint << " ";

    // Output file name.
    cmd << STR_DXC_OPTION_OUTPUT_FILE << " \"" << outputFileName << "\" ";

    // Include paths.
    for (const std::string& includePath : includePaths)
    {
        // DXC doesn't handle it when there is a '\' at the end of the path.
        std::string fixedIncludePath;
        FixIncludePath(includePath, fixedIncludePath);
        cmd << STR_DXC_OPTION_INCLUDE_PATH << " \"" << fixedIncludePath << "\" ";
    }

    // Preprocessor defines.
    for (const std::string& prepreocessorDefine : prepreocessorDefine)
    {
        cmd << STR_DXC_OPTION_PREPROCESSOR_DEFINES << " " << prepreocessorDefine << " ";
    }

    // DXIL disassembly.
    if (!config.m_csDxilDisassembly.empty())
    {
        cmd << STR_DXC_OPTION_DXIL_DISASSEMBLY_OUTPUT_FILE << " \"" << config.m_csDxilDisassembly << "\" ";
    }

    // Shader HLSL file.
    cmd << "\"" << shaderHlsl << "\"";

    long        exitCode = 0;
    osFilePath  dxcExe;
    osGetCurrentApplicationPath(dxcExe, false);
    dxcExe.appendSubDirectory(DX12_DXC_DIR);
    dxcExe.setFileName(DX12_DXC_EXE);

    // Clear the error message buffer.
    dxcOutput.clear();
    dxcErrors.clear();

    kcUtils::ProcessStatus status = kcUtils::LaunchProcess(dxcExe.asString().asASCIICharArray(),
        cmd.str(),
        "",
        PROCESS_WAIT_INFINITE,
        config.m_printProcessCmdLines,
        dxcOutput,
        dxcErrors,
        exitCode);

    assert(status == kcUtils::ProcessStatus::Success);
    ret = (status == kcUtils::ProcessStatus::Success ? beStatus_SUCCESS : beStatus_dx12BackendLaunchFailure);

    return ret;
}

bool CompileHlslWithDxc(const Config& config, const std::string& hlslFile, bePipelineStage stage, const std::string& shaderModel,
    const std::string& entryPoint, std::string& dxilOutputFile)
{
    bool ret = false;
    const char* STR_DXIL_FILE_NAME = "dxil";
    const char* STR_DXIL_FILE_SUFFIX = "obj";
    bool shouldAbort = !kcUtils::ConstructOutFileName("", STR_DX12_STAGE_SUFFIX[stage],
        STR_DXIL_FILE_NAME, STR_DXIL_FILE_SUFFIX, dxilOutputFile);
    assert(!shouldAbort);
    if (!shouldAbort)
    {
        // Notify the user.
        std::cout << STR_INFO_FRONT_END_COMPILATION_WITH_DXC;

        // Perform the front-end compilation through DXC.
        std::string dxcOutput;
        std::string dxcErrors;
        bool isLaunchSuccessful = InvokDxc(config, hlslFile, shaderModel,
            entryPoint, config.m_IncludePath, config.m_Defines, dxilOutputFile, dxcOutput, dxcErrors);
        assert(isLaunchSuccessful);
        bool isDxcTextOutputAvailable = !dxcOutput.empty() || !dxcErrors.empty();
        if (isDxcTextOutputAvailable)
        {
            // Inform user that output from DXC is coming.
            std::cout << std::endl << STR_INFO_DXC_OUTPUT_PROLOGUE << std::endl << std::endl;
        }
        if (!dxcOutput.empty())
        {
            // Print output.
            std::cout << dxcOutput << std::endl;
        }
        if (!dxcErrors.empty())
        {
            // Print errors.
            std::cout << dxcErrors << std::endl;
        }
        if (isDxcTextOutputAvailable)
        {
            // Print DXC output epilogue.
            std::cout << STR_INFO_DXC_OUTPUT_EPILOGUE << std::endl;
        }

        if (!isLaunchSuccessful)
        {
            // If no error messages printed by
            // front-end compiler, assume failure to launch.
            if (dxcOutput.empty() && dxcErrors.empty())
            {
                std::cout << std::endl << STR_ERROR_DXC_LAUNCH_FAILED << std::endl;
            }
        }
        else
        {
            bool isDxilFileValid = kcUtils::FileNotEmpty(dxilOutputFile);
            if (isDxilFileValid)
            {
                std::cout << STR_INFO_FRONT_END_COMPILATION_SUCCESS << std::endl;
                ret = true;
            }
            else
            {
                std::cout << STR_ERROR_HLSL_TO_DXIL_COMPILATION_FAILED_A <<
                    STR_DX12_STAGE_NAMES[stage] << STR_ERROR_HLSL_TO_DXIL_COMPILATION_FAILED_B << std::endl;
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
static bool IsLegacyShaderModel(const std::string& shaderModel, bool& isLegacy)
{
    bool ret = true;
    std::vector<std::string> modelComponents;
    beUtils::splitString(shaderModel, '_', modelComponents);
    assert(modelComponents.size() > 2);
    if (modelComponents.size() > 2)
    {
        bool isNumericValue = beUtils::IsNumericValue(modelComponents[1]);
        assert(isNumericValue);
        if (isNumericValue)
        {
            int versionMajor = std::stoi(modelComponents[1]);
            if (versionMajor < 5)
            {
                // If shader model is lower than 5, it's legacy.
                isLegacy = true;
            }
            else if (versionMajor == 5)
            {
                bool isNumericValue = beUtils::IsNumericValue(modelComponents[2]);
                assert(isNumericValue);
                if (isNumericValue)
                {
                    int versionMinor = std::stoi(modelComponents[2]);
                    if (versionMinor == 0)
                    {
                        // If shader model is 5.0, it's legacy.
                        isLegacy = true;
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

    return ret;
}

bool beUtils::IsNumericValue(const std::string& str)
{
    return !str.empty() && std::find_if(str.begin(),
        str.end(), [](char c) { return !std::isdigit(c); }) == str.end();
}

static beStatus InvokeDx12Backend(const std::string& cmdLineOptions, bool printCmd,
    std::string& outText, std::string& errorMsg)
{
    osFilePath  dx12BackendExe;
    long        exitCode = 0;

    osGetCurrentApplicationPath(dx12BackendExe, false);
    dx12BackendExe.appendSubDirectory(DX12_BACKEND_DIR);

    dx12BackendExe.setFileName(DX12_BACKEND_EXE);

    // Clear the error message buffer.
    errorMsg.clear();

    kcUtils::ProcessStatus status = kcUtils::LaunchProcess(dx12BackendExe.asString().asASCIICharArray(),
        cmdLineOptions,
        "",
        PROCESS_WAIT_INFINITE,
        printCmd,
        outText,
        errorMsg,
        exitCode);

    assert(status == kcUtils::ProcessStatus::Success);
    return (status == kcUtils::ProcessStatus::Success ? beStatus_SUCCESS : beStatus_dx12BackendLaunchFailure);
}


// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

beKA::beStatus beProgramBuilderDX12::GetSupportGpus(const Config& config,
    std::vector<std::string>& gpus, std::map<std::string, int>& driverIds)
{
    driverIds.clear();
    std::string errors;

    // Retrieve the list of targets.
    std::string supportedGpus;
    beStatus ret = InvokeDx12Backend("-l", config.m_printProcessCmdLines, supportedGpus, errors);
    assert(ret = beStatus::beStatus_SUCCESS);
    assert(!supportedGpus.empty());
    if ((ret == beStatus::beStatus_SUCCESS) && !supportedGpus.empty())
    {
        // Post-process the list.
        std::vector<std::string> splitGpuNames;
        beUtils::splitString(supportedGpus, '\n', splitGpuNames);
        assert(!splitGpuNames.empty());
        if (!splitGpuNames.empty())
        {
            // Name format is: <codename>:<gfxName>-><driver id>. We want both codename and gfxName,
            // then they would be passed to the code that compares them to known gpu names and filters as necessary.
            for (const std::string& driverName : splitGpuNames)
            {
                // Break by ':'.
                std::vector<std::string> splitNamesColon;
                beUtils::splitString(driverName, ':', splitNamesColon);
                assert(!splitNamesColon.empty());
                if (!splitNamesColon.empty())
                {
                    std::transform(splitNamesColon[0].begin(),
                        splitNamesColon[0].end(), splitNamesColon[0].begin(), ::tolower);

                    // Check if the name needs to be corrected to the "standard" name used by this tool.
                    auto correctedName = std::find_if(PAL_DEVICE_NAME_MAPPING.cbegin(), PAL_DEVICE_NAME_MAPPING.cend(),
                        [&](const std::pair<std::string, std::string>& device) { return (device.first == splitNamesColon[0]); });

                    if (correctedName == PAL_DEVICE_NAME_MAPPING.end())
                    {
                        gpus.push_back(splitNamesColon[0]);
                    }
                    else
                    {
                        gpus.push_back(correctedName->second);
                    }

                    assert(splitNamesColon.size() > 1);
                    if (splitNamesColon.size() > 1)
                    {
                        // Break by '->'.
                        std::vector<std::string> splitNamesArrow;
                        beUtils::splitString(splitNamesColon[1], '-', splitNamesArrow);
                        assert(!splitNamesArrow.empty());
                        if (!splitNamesArrow.empty())
                        {
                            std::transform(splitNamesArrow[0].begin(),
                                splitNamesArrow[0].end(), splitNamesArrow[0].begin(), ::tolower);
                            gpus.push_back(splitNamesArrow[0]);

                            // Get the id for this device.
                            std::vector<std::string> splitTriangularBracket;
                            beUtils::splitString(splitNamesArrow[1], '>', splitTriangularBracket);
                            assert(splitTriangularBracket.size() > 1);
                            if (splitTriangularBracket.size() > 1)
                            {
                                // Track the ID for both codename and gfx name.
                                int id = std::stoi(splitTriangularBracket[1], nullptr);
                                driverIds[splitNamesColon[0]] = id;
                                driverIds[splitNamesArrow[0]] = id;

                                // If we corrected the name, track the corrected codename as well.
                                if (correctedName != PAL_DEVICE_NAME_MAPPING.end())
                                {
                                    driverIds[correctedName->second] = id;
                                }
                            }
                        }
                    }
                }
            }

            assert(!gpus.empty());
            std::vector<GDT_GfxCardInfo> cardList;
            std::set<std::string> knownArchNames;
            // Get the list of known GPU architectures from DeviceInfo.
            bool isAllCardsExtracted = beUtils::GetAllGraphicsCards(cardList, knownArchNames, true);
            assert(isAllCardsExtracted);
            if (isAllCardsExtracted)
            {
                for (auto iter = gpus.begin(); iter != gpus.end();)
                {
                    if (std::find(knownArchNames.begin(), knownArchNames.end(), (*iter)) == knownArchNames.end())
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
        std::cout << STR_ERROR_FAILED_TO_INVOKE_DX12_BACKEND << std::endl;
    }

    return ret;
}


bool HandleHlslArgument(const Config &config, bePipelineStage stage,
    const char* stageCmdName, const std::string& hlslFile,
    const std::string& shaderModel, const std::string& entryPoint,
    bool& isLegacyShaderModel, std::stringstream &cmd, std::string &dxilCompiled)
{
    bool ret = false;
    bool isShaderModelValid = IsLegacyShaderModel(shaderModel, isLegacyShaderModel);
    assert(isShaderModelValid);
    if (isShaderModelValid)
    {
        if (isLegacyShaderModel)
        {
            cmd << "--" << stageCmdName << " " << "\"" << hlslFile << "\" ";
            ret = true;
        }
        else
        {
            // Compile the HLSL file through DXC.
            ret = CompileHlslWithDxc(config, hlslFile,
                stage, shaderModel, entryPoint, dxilCompiled);
            if (ret)
            {
                // Pass the compilation output to the backend as a binary.
                cmd << "--" << stageCmdName << "-dxbc " << "\"" << dxilCompiled << "\" ";
            }
        }
    }
    else
    {
        std::cout << STR_ERROR_INVALID_SHADER_MODEL << shaderModel << std::endl;
    }
    return ret;
}

beKA::beStatus beProgramBuilderDX12::Compile(const Config& config, const std::string& targetDevice,
    std::string& outText, std::string& errorMsg, beVkPipelineFiles& generatedIsaFiles,
    beVkPipelineFiles& generatedStatFiles)
{
    beKA::beStatus ret = beStatus::beStatus_Invalid;
    std::stringstream cmd;

    if (m_codeNameToDriverId.empty())
    {
        std::vector<std::string> gpus;
        GetSupportGpus(config, gpus, m_codeNameToDriverId);
    }
    assert(!m_codeNameToDriverId.empty());

    if (!m_codeNameToDriverId.empty())
    {
        // Target device.
        auto iter = m_codeNameToDriverId.find(targetDevice);
        if (iter != m_codeNameToDriverId.end())
        {
            const wchar_t* STR_ENV_VAR_NAME = L"AmdVirtualGpuId";
            std::wstring value = std::to_wstring(iter->second);

            // Set the environment variable.
            BOOL rc = SetEnvironmentVariable(STR_ENV_VAR_NAME, value.c_str());
            assert(rc == TRUE);
            if (rc == TRUE)
            {
                // True if we should abort the compilation process.
                bool shouldAbort = false;

                // For each stage, check if the target profile is of a legacy shader model (5.0 or below).
                // If this is the case, we will pass the backend the required parameters to perform the
                // compilation through the runtime into DXBC. Otherwise, we will compile first through DXC
                // into DXIL, and then use the generated binary as a pre-compiled input for the backend.
                bool isLegacyShaderModelVs = false;
                bool isLegacyShaderModelHs = false;
                bool isLegacyShaderModelDs = false;
                bool isLegacyShaderModelGs = false;
                bool isLegacyShaderModelPs = false;
                bool isLegacyShaderModelCs = false;

                // Names of DXIL files generated by this stage in
                // case that the shader model is not a legacy one.
                std::string dxilCompiledVs;
                std::string dxilCompiledHs;
                std::string dxilCompiledDs;
                std::string dxilCompiledGs;
                std::string dxilCompiledPs;
                std::string dxilCompiledCs;

                // Input files - HLSL.
                if (!shouldAbort && !config.m_vsHlsl.empty())
                {
                    shouldAbort = !HandleHlslArgument(config, bePipelineStage::Geometry, "vert", config.m_vsHlsl,
                        config.m_vsModel, config.m_vsEntryPoint, isLegacyShaderModelVs, cmd, dxilCompiledVs);
                }
                if (!shouldAbort && !config.m_hsHlsl.empty())
                {
                    shouldAbort = !HandleHlslArgument(config, bePipelineStage::TessellationControl, "hull", config.m_hsHlsl,
                        config.m_hsModel, config.m_hsEntryPoint, isLegacyShaderModelHs, cmd, dxilCompiledHs);
                }
                if (!shouldAbort && !config.m_dsHlsl.empty())
                {
                    shouldAbort = !HandleHlslArgument(config, bePipelineStage::TessellationEvaluation, "domain", config.m_dsHlsl,
                        config.m_dsModel, config.m_dsEntryPoint, isLegacyShaderModelDs, cmd, dxilCompiledDs);
                }
                if (!shouldAbort && !config.m_gsHlsl.empty())
                {
                    shouldAbort = !HandleHlslArgument(config, bePipelineStage::Geometry, "geom", config.m_gsHlsl,
                        config.m_gsModel, config.m_gsEntryPoint, isLegacyShaderModelGs, cmd, dxilCompiledGs);
                }
                if (!shouldAbort && !config.m_psHlsl.empty())
                {
                    shouldAbort = !HandleHlslArgument(config, bePipelineStage::Fragment, "pixel", config.m_psHlsl,
                        config.m_psModel, config.m_psEntryPoint, isLegacyShaderModelPs, cmd, dxilCompiledPs);
                }
                if (!shouldAbort && !config.m_csHlsl.empty())
                {
                    shouldAbort = !HandleHlslArgument(config, bePipelineStage::Compute, "comp", config.m_csHlsl,
                        config.m_csModel, config.m_csEntryPoint, isLegacyShaderModelCs, cmd, dxilCompiledCs);

                    // Special case: if we switched HLSL input with binary input and RS is in HLSL file and
                    // needs to be compiled from a macro, and if an HLSL file has not been given by the user since
                    // this is a compute shader which only requires a single HLSL path, pass the HLSL path option
                    // so that the backend knows where to read the macro from.
                    if (!dxilCompiledCs.empty() && !config.m_rsMacro.empty() && config.m_rsHlsl.empty())
                    {
                        cmd << " --rs-hlsl " << "\"" << config.m_csHlsl << "\" ";
                    }
                }

                if (!shouldAbort)
                {
                    // Input files - DXBC.
                    if (!config.m_vsDxbc.empty())
                    {
                        cmd << "--vert-dxbc " << "\"" << config.m_vsDxbc << "\" ";
                    }
                    if (!config.m_hsDxbc.empty())
                    {
                        cmd << "--hull-dxbc " << "\"" << config.m_hsDxbc << "\" ";
                    }
                    if (!config.m_dsDxbc.empty())
                    {
                        cmd << "--domain-dxbc " << "\"" << config.m_dsDxbc << "\" ";
                    }
                    if (!config.m_gsDxbc.empty())
                    {
                        cmd << "--geom-dxbc " << "\"" << config.m_gsDxbc << "\" ";
                    }
                    if (!config.m_psDxbc.empty())
                    {
                        cmd << "--pixel-dxbc " << "\"" << config.m_psDxbc << "\" ";
                    }
                    if (!config.m_csDxbc.empty())
                    {
                        cmd << "--comp-dxbc " << "\"" << config.m_csDxbc << "\" ";
                    }

                    // Entry point.
                    if (!config.m_vsEntryPoint.empty())
                    {
                        cmd << "--vert-entry " << config.m_vsEntryPoint << " ";
                    }
                    if (!config.m_hsEntryPoint.empty())
                    {
                        cmd << "--hull-entry " << config.m_hsEntryPoint << " ";
                    }
                    if (!config.m_dsEntryPoint.empty())
                    {
                        cmd << "--domain-entry " << config.m_dsEntryPoint << " ";
                    }
                    if (!config.m_gsEntryPoint.empty())
                    {
                        cmd << "--geom-entry " << config.m_gsEntryPoint << " ";
                    }
                    if (!config.m_psEntryPoint.empty())
                    {
                        cmd << "--pixel-entry " << config.m_psEntryPoint << " ";
                    }
                    if (!config.m_csEntryPoint.empty())
                    {
                        cmd << "--comp-entry " << config.m_csEntryPoint << " ";
                    }

                    // Shader model.
                    if (!config.m_vsModel.empty())
                    {
                        cmd << "--vert-target " << config.m_vsModel << " ";
                    }
                    if (!config.m_hsModel.empty())
                    {
                        cmd << "--hull-target " << config.m_hsModel << " ";
                    }
                    if (!config.m_dsModel.empty())
                    {
                        cmd << "--domain-target " << config.m_dsModel << " ";
                    }
                    if (!config.m_gsModel.empty())
                    {
                        cmd << "--geom-target " << config.m_gsModel << " ";
                    }
                    if (!config.m_psModel.empty())
                    {
                        cmd << "--pixel-target " << config.m_psModel << " ";
                    }
                    if (!config.m_csModel.empty())
                    {
                        cmd << "--comp-target " << config.m_csModel << " ";
                    }

                    // Root signature.
                    if (!config.m_rsBin.empty())
                    {
                        cmd << "--rs-bin " << "\"" << config.m_rsBin << "\" ";
                    }
                    if (!config.m_rsHlsl.empty())
                    {
                        cmd << "--rs-hlsl " << "\"" << config.m_rsHlsl << "\" ";
                    }
                    if (!config.m_rsMacro.empty())
                    {
                        cmd << "--rs-macro " << "\"" << config.m_rsMacro << "\" ";
                    }
                    if (!config.m_rsMacroVersion.empty())
                    {
                        cmd << "--rs-macro-version " << "\"" << config.m_rsMacroVersion << "\" ";
                    }

                    // Include.
                    for (const std::string& includePath : config.m_IncludePath)
                    {
                        std::string fixedIncludePath;
                        FixIncludePath(includePath, fixedIncludePath);
                        cmd << "--include " << "\"" << fixedIncludePath << "\" ";
                    }

                    // Preprocessor defines.
                    for (const std::string& includePath : config.m_Defines)
                    {
                        cmd << "--define " << includePath << " ";
                    }

                    // Package the ISA and statistics file names in separate containers for pre-processing.
                    std::string inputFiles[bePipelineStage::Count] = { config.m_vsHlsl, config.m_hsHlsl,
                        config.m_dsHlsl, config.m_gsHlsl, config.m_psHlsl, config.m_csHlsl };

                    // If DXBC is used as an input instead of HLSL, replace the input file.
                    if (inputFiles[bePipelineStage::Vertex].empty() && !config.m_vsDxbc.empty())
                    {
                        inputFiles[bePipelineStage::Vertex] = config.m_vsDxbc;
                    }
                    if (inputFiles[bePipelineStage::TessellationControl].empty() && !config.m_hsDxbc.empty())
                    {
                        inputFiles[bePipelineStage::TessellationControl] = config.m_hsDxbc;
                    }
                    if (inputFiles[bePipelineStage::TessellationEvaluation].empty() && !config.m_dsDxbc.empty())
                    {
                        inputFiles[bePipelineStage::TessellationEvaluation] = config.m_dsDxbc;
                    }
                    if (inputFiles[bePipelineStage::Geometry].empty() && !config.m_gsDxbc.empty())
                    {
                        inputFiles[bePipelineStage::Geometry] = config.m_gsDxbc;
                    }
                    if (inputFiles[bePipelineStage::Fragment].empty() && !config.m_psDxbc.empty())
                    {
                        inputFiles[bePipelineStage::Fragment] = config.m_psDxbc;
                    }
                    if (inputFiles[bePipelineStage::Compute].empty() && !config.m_csDxbc.empty())
                    {
                        inputFiles[bePipelineStage::Compute] = config.m_csDxbc;
                    }
                    for (int stage = 0; stage < bePipelineStage::Count; stage++)
                    {
                        if (!inputFiles[stage].empty())
                        {
                            // ISA files.
                            if (!config.m_ISAFile.empty())
                            {
                                bool isFileNameConstructed = kcUtils::ConstructOutFileName(config.m_ISAFile,
                                    STR_DX12_STAGE_SUFFIX[stage], targetDevice, "isa", generatedIsaFiles[stage]);
                                assert(isFileNameConstructed);
                                if (isFileNameConstructed && !generatedIsaFiles[stage].empty())
                                {
                                    cmd << " --" << STR_DX12_STAGE_SUFFIX[stage] << "-isa " << "\"" << generatedIsaFiles[stage] << "\" ";

                                    // Delete that file if it already exists.
                                    if (beUtils::IsFilePresent(generatedIsaFiles[stage]))
                                    {
                                        beUtils::DeleteFileFromDisk(generatedIsaFiles[stage]);
                                    }
                                }
                            }

                            // Statistics files.
                            if (!config.m_AnalysisFile.empty())
                            {
                                bool isFileNameConstructed = kcUtils::ConstructOutFileName(config.m_AnalysisFile,
                                    STR_DX12_STAGE_SUFFIX[stage], targetDevice,
                                    "csv", generatedStatFiles[stage]);
                                assert(isFileNameConstructed);
                                if (isFileNameConstructed && !generatedStatFiles[stage].empty())
                                {
                                    cmd << " --" << STR_DX12_STAGE_SUFFIX[stage] << "-stats " << "\"" << generatedStatFiles[stage] << "\" ";
                                }

                                // Delete that file if it already exists.
                                if (beUtils::IsFilePresent(generatedIsaFiles[stage]))
                                {
                                    beUtils::DeleteFileFromDisk(generatedIsaFiles[stage]);
                                }
                            }
                        }
                    }

                    // For older shader models, we should retrieve the DXBC disassembly from the runtime compiler.
                    // For the newer shader models, this has already been done offline through DXC.
                    bool isLegacyShaderModel = false;
                    if (!config.m_vsHlsl.empty() && !config.m_vsDxilDisassembly.empty() &&
                        IsLegacyShaderModel(config.m_vsModel, isLegacyShaderModel) &&
                        isLegacyShaderModel)
                    {
                        cmd << "--vert-dxbc-dis \"" << config.m_vsDxilDisassembly << "\" ";
                    }
                    if (!config.m_hsHlsl.empty() && !config.m_hsDxilDisassembly.empty() &&
                        IsLegacyShaderModel(config.m_hsModel, isLegacyShaderModel) &&
                        isLegacyShaderModel)
                    {
                        cmd << "--hull-dxbc-dis \"" << config.m_hsDxilDisassembly << "\" ";
                    }
                    if (!config.m_dsHlsl.empty() && !config.m_dsDxilDisassembly.empty() &&
                        IsLegacyShaderModel(config.m_dsModel, isLegacyShaderModel) &&
                        isLegacyShaderModel)
                    {
                        cmd << "--domain-dxbc-dis \"" << config.m_dsDxilDisassembly << "\" ";
                    }
                    if (!config.m_gsHlsl.empty() && !config.m_gsDxilDisassembly.empty() &&
                        IsLegacyShaderModel(config.m_gsModel, isLegacyShaderModel) &&
                        isLegacyShaderModel)
                    {
                        cmd << "--geom-dxbc-dis \"" << config.m_gsDxilDisassembly << "\" ";
                    }
                    if (!config.m_psHlsl.empty() && !config.m_psDxilDisassembly.empty() &&
                        IsLegacyShaderModel(config.m_psModel, isLegacyShaderModel) &&
                        isLegacyShaderModel)
                    {
                        cmd << "--pixel-dxbc-dis \"" << config.m_psDxilDisassembly << "\" ";
                    }
                    if (!config.m_csHlsl.empty() && !config.m_csDxilDisassembly.empty() &&
                        IsLegacyShaderModel(config.m_csModel, isLegacyShaderModel) &&
                        isLegacyShaderModel)
                    {
                        cmd << "--comp-dxbc-dis \"" << config.m_csDxilDisassembly << "\" ";
                    }

                    ret = InvokeDx12Backend(cmd.str().c_str(), config.m_printProcessCmdLines, outText, errorMsg);
                    assert(ret == beStatus_SUCCESS);
                    if (ret == beStatus_SUCCESS)
                    {
                        const char* STR_DX12_BACKEND_ERROR_TOKEN = "Error";
                        bool isSuccess = outText.find(STR_DX12_BACKEND_ERROR_TOKEN) == std::string::npos &&
                            errorMsg.find(STR_DX12_BACKEND_ERROR_TOKEN) == std::string::npos;
                        if (!isSuccess)
                        {
                            ret = beStatus_dx12CompileFailure;
                        }
                    }
                    else
                    {
                        if (!errorMsg.empty())
                        {
                            std::cout << errorMsg << std::endl;
                        }
                        std::cout << STR_ERROR_FAILED_TO_INVOKE_DX12_BACKEND << std::endl;
                    }

                    // Cleanup: delete temporary files (DXIL files that were generated by DXC).
                    if (kcUtils::FileNotEmpty(dxilCompiledVs))
                    {
                        kcUtils::DeleteFile(dxilCompiledVs);
                    }
                    if (kcUtils::FileNotEmpty(dxilCompiledHs))
                    {
                        kcUtils::DeleteFile(dxilCompiledHs);
                    }
                    if (kcUtils::FileNotEmpty(dxilCompiledDs))
                    {
                        kcUtils::DeleteFile(dxilCompiledDs);
                    }
                    if (kcUtils::FileNotEmpty(dxilCompiledGs))
                    {
                        kcUtils::DeleteFile(dxilCompiledGs);
                    }
                    if (kcUtils::FileNotEmpty(dxilCompiledPs))
                    {
                        kcUtils::DeleteFile(dxilCompiledPs);
                    }
                    if (kcUtils::FileNotEmpty(dxilCompiledCs))
                    {
                        kcUtils::DeleteFile(dxilCompiledCs);
                    }
                }
            }
            else
            {
                std::cout << STR_ERROR_FAILED_TO_SET_ENV_VAR << std::endl;
            }
        }
        else
        {
            std::cout << STR_ERROR_NO_SUPPORTED_TARGET_A <<
                targetDevice << STR_ERROR_NO_SUPPORTED_TARGET_B << std::endl;
        }
    }
    else
    {
        std::cout << STR_ERROR_CANNOT_RETRIEVE_SUPPORTED_TARGET_LIST << std::endl;
    }

    return ret;
}

#endif
