//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#include <map>
#include <utility>
#include <sstream>
#include <boost/algorithm/string.hpp>

// Infra.
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderOpenCL.h>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/src/kcParseCmdLine.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderCL.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderOpenGL.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderVulkan.h>
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>

#ifdef _WIN32
    #include <RadeonGPUAnalyzerCLI/src/kcCLICommanderDX.h>
#endif

using namespace beKA;
static std::ostream& s_Log = cout;

static void loggingCallback(const string& s)
{
    s_Log << s.c_str();
}

static void PrintAsicList()
{
    // We do not want to display names that contain these strings.
    const char* FILTER_INDICATOR_1 = ":";
    const char* FILTER_INDICATOR_2 = "Not Used";

    std::map<std::string, std::set<std::string>> cardsMapping;
    bool rc = kcUtils::GetMarketingNameToCodenameMapping(cardsMapping);
    if (rc && !cardsMapping.empty())
    {
        for (const auto& pair : cardsMapping)
        {
            s_Log << pair.first << std::endl;
            for (const std::string& card : pair.second)
            {
                // Filter out internal names.
                if (card.find(FILTER_INDICATOR_1) == std::string::npos &&
                    card.find(FILTER_INDICATOR_2) == std::string::npos)
                {
                    s_Log << "\t" << card << std::endl;
                }
            }
        }
    }
    else
    {
        s_Log << STR_ERR_CANNOT_EXTRACT_SUPPORTED_DEVICE_LIST << std::endl;
        exit(-1);
    }
}

int main(int argc, char* argv[])
{
#ifdef _WIN64
    // Enable 64-bit build for OpenCL.
    osEnvironmentVariable envVar64Bit(STR_OCL_ENV_VAR_GPU_FORCE_64BIT_PTR_NAME, STR_OCL_ENV_VAR_GPU_FORCE_64BIT_PTR_VALUE);
    osSetCurrentProcessEnvVariable(envVar64Bit);
#endif // _WIN64

    Config config;
    bool bCont = ParseCmdLine(argc, argv, config);

    if (!bCont)
    {
        config.m_RequestedCommand = Config::ccInvalid;
    }

    // do requested work
    kcCLICommander* pMyCommander = nullptr;

    if (config.m_SourceLanguage == SourceLanguage_OpenCL)
    {
        pMyCommander = reinterpret_cast<kcCLICommander*>(new kcCLICommanderCL);
    }

#ifdef _WIN32
    else if ((config.m_SourceLanguage == SourceLanguage_HLSL)   ||
             (config.m_SourceLanguage == SourceLanguage_AMDIL)  ||
             (config.m_SourceLanguage == SourceLanguage_DXasm)  ||
             (config.m_SourceLanguage == SourceLanguage_DXasmT))
    {
        pMyCommander = reinterpret_cast<kcCLICommander*>(new kcCLICommanderDX);
    }

#endif

    else if (config.m_SourceLanguage == SourceLanguage_GLSL)
    {
        s_Log << STR_ERR_GLSL_MODE_DEPRECATED << std::endl;
        exit(-1);
    }
    else if (config.m_SourceLanguage == SourceLanguage_GLSL_OpenGL)
    {
        pMyCommander = reinterpret_cast<kcCLICommander*>(new kcCLICommanderOpenGL);
    }
    else if (config.m_SourceLanguage == SourceLanguage_GLSL_Vulkan || 
        config.m_SourceLanguage == SourceLanguage_SPIRV_Vulkan ||
        config.m_SourceLanguage == SourceLanguage_SPIRVTXT_Vulkan)
    {
        pMyCommander = reinterpret_cast<kcCLICommander*>(new kcCLICommanderVulkan);
    }

    if (pMyCommander == nullptr)
    {
        config.m_RequestedCommand = Config::ccInvalid;
    }

    switch (config.m_RequestedCommand)
    {
        case Config::ccHelp:
            break;

        case Config::ccCompile:
        case Config::ccListKernels:
            pMyCommander->RunCompileCommands(config, loggingCallback);
            break;

        case Config::ccListAsics:
            PrintAsicList();
            break;

        case Config::ccVersion:
            pMyCommander->Version(config, loggingCallback);
            break;

        case Config::ccInvalid:
            s_Log << STR_ERR_NO_VALID_CMD_DETECTED << endl;
            break;
    }

    delete pMyCommander;

    return 0;
}
