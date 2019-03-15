// C++.
#include <sstream>
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgCliUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>

// Common between CLI and GUI.
#include <RadeonGPUAnalyzerGUI/../Utils/Include/rgaCliDefs.h>


bool rgCliUtils::GenerateOpenClBuildSettingsString(const rgBuildSettingsOpenCL& buildSettings, std::string& str, bool additionalOptions)
{
    bool ret = false;
    std::stringstream cmd;

    // Include paths.
    for (std::string includePath : buildSettings.m_additionalIncludeDirectories)
    {
        // Remove trailing slash symbols.
        if (!includePath.empty() && (includePath[includePath.size() - 1] == '\\' || includePath[includePath.size() - 1] == '/'))
        {
            includePath.erase(includePath.size() - 1, 1);
        }
        cmd << CLI_OPT_ADDITIONAL_INCLUDE_PATH << " \"" << includePath << "\" ";
    }

    // Preprocessor directives.
    for (const std::string& currDirective : buildSettings.m_predefinedMacros)
    {
        cmd << CLI_OPT_PREPROCESSOR_DIRECTIVE << " " << currDirective << " ";
    }

    // Accumulate the OpenCL-specific options in a separate stream.
    std::stringstream clSpecificOptions;

    // OpenCL-specific build options.
    if (buildSettings.m_isAggressiveMathOptimizations)
    {
        clSpecificOptions << CLI_OPT_CL_AGGRESSIVE_OPTIMIZATIONS << " ";
    }
    if (buildSettings.m_isCorrectlyRoundDivSqrt)
    {
        clSpecificOptions << CLI_OPT_CL_CORRECT_ROUND_DIV_SQRT << " ";
    }
    if (buildSettings.m_isNoNanNorInfinite)
    {
        clSpecificOptions << CLI_OPT_CL_IS_NAN_OR_INIFINITE << " ";
    }
    if (buildSettings.m_isUnsafeOptimizations)
    {
        clSpecificOptions << CLI_OPT_CL_UNSAFE_OPTIMIZATIONS << " ";
    }
    if (buildSettings.m_isIgnoreZeroSignedness)
    {
        clSpecificOptions << CLI_OPT_CL_IGNORE_ZERO_SIGNEDNESS << " ";
    }
    if (buildSettings.m_isEnableMAD)
    {
        clSpecificOptions << CLI_OPT_CL_ENABLE_MAD << " ";
    }
    if (buildSettings.m_isStrictAliasing)
    {
        clSpecificOptions << CLI_OPT_CL_STRICT_ALIASING << " ";
    }
    if (buildSettings.m_isDenormsAsZeros)
    {
        clSpecificOptions << CLI_OPT_CL_DENORMS_AS_ZEROES << " ";
    }
    if (buildSettings.m_isTreatDoubleAsSingle)
    {
        clSpecificOptions << CLI_OPT_CL_TREAT_DOUBLE_AS_SINGLE << " ";
    }

    // Append the additional options if required.
    if (additionalOptions && !buildSettings.m_additionalOptions.empty())
    {
        clSpecificOptions << buildSettings.m_additionalOptions;
    }

    // Add the OpenCL-specific options, if there are any.
    if (!clSpecificOptions.str().empty())
    {
        cmd << CLI_OPT_CL_OPTION << " \"" << clSpecificOptions.str() << "\" ";
    }

    // Add the alternative compiler paths.
    if (!std::get<CompilerFolderType::Bin>(buildSettings.m_compilerPaths).empty())
    {
        cmd << CLI_OPT_COMPILER_BIN_DIR << " \"" << std::get<CompilerFolderType::Bin>(buildSettings.m_compilerPaths) << "\" ";
    }
    if (!std::get<CompilerFolderType::Include>(buildSettings.m_compilerPaths).empty())
    {
        cmd << CLI_OPT_COMPILER_INC_DIR << " \"" << std::get<CompilerFolderType::Include>(buildSettings.m_compilerPaths) << "\" ";
    }
    if (!std::get<CompilerFolderType::Lib>(buildSettings.m_compilerPaths).empty())
    {
        cmd << CLI_OPT_COMPILER_LIB_DIR << " \"" << std::get<CompilerFolderType::Lib>(buildSettings.m_compilerPaths) << "\" ";
    }

    if (buildSettings.m_optimizationLevel != buildSettings.OPENCL_DEFAULT_OPT_LEVEL)
    {
        // Tokens to identify the user's selection.
        const char* OPTIMIZATION_LEVEL_0_TOKEN = "O0";
        const char* OPTIMIZATION_LEVEL_1_TOKEN = "O1";
        const char* OPTIMIZATION_LEVEL_2_TOKEN = "O2";
        const char* OPTIMIZATION_LEVEL_3_TOKEN = "O3";

        if (buildSettings.m_optimizationLevel.find(OPTIMIZATION_LEVEL_0_TOKEN) != std::string::npos)
        {
            cmd << CLI_OPT_CL_OPTMIZATION_LEVEL_0 << " ";
        }
        else if (buildSettings.m_optimizationLevel.find(OPTIMIZATION_LEVEL_1_TOKEN) != std::string::npos)
        {
            cmd << CLI_OPT_CL_OPTMIZATION_LEVEL_1 << " ";
        }
        else if (buildSettings.m_optimizationLevel.find(OPTIMIZATION_LEVEL_2_TOKEN) != std::string::npos)
        {
            cmd << CLI_OPT_CL_OPTMIZATION_LEVEL_2 << " ";
        }
        else if (buildSettings.m_optimizationLevel.find(OPTIMIZATION_LEVEL_3_TOKEN) != std::string::npos)
        {
            cmd << CLI_OPT_CL_OPTMIZATION_LEVEL_3 << " ";
        }
    }

    str = cmd.str();
    ret = true;

    return ret;
}

bool rgCliUtils::GenerateVulkanBuildSettingsString(const rgBuildSettingsVulkan& buildSettings, std::string& str, bool additionalOptions)
{
    bool ret = false;
    std::stringstream cmd;

    // Include paths.
    for (std::string includePath : buildSettings.m_additionalIncludeDirectories)
    {
        // Remove trailing slash symbols.
        if (!includePath.empty() && (includePath[includePath.size() - 1] == '\\' || includePath[includePath.size() - 1] == '/'))
        {
            includePath.erase(includePath.size() - 1, 1);
        }
        cmd << CLI_OPT_ADDITIONAL_INCLUDE_PATH << " \"" << includePath << "\" ";
    }

    // Preprocessor directives.
    for (const std::string& currDirective : buildSettings.m_predefinedMacros)
    {
        cmd << CLI_OPT_PREPROCESSOR_DIRECTIVE << " " << currDirective << " ";
    }

    // Accumulate the Vulkan-specific options in a separate stream.
    std::stringstream vulkanSpecificOptions;

    // Vulkan-specific build options.
    if (buildSettings.m_isGenerateDebugInfoChecked)
    {
        vulkanSpecificOptions << CLI_OPT_VULKAN_GENERATE_DEBUG_INFORMATION << " ";
    }
    if (buildSettings.m_isNoExplicitBindingsChecked)
    {
        vulkanSpecificOptions << CLI_OPT_VULKAN_NO_EXPLICIT_BINDINGS << " ";
    }
    if (buildSettings.m_isUseHlslBlockOffsetsChecked)
    {
        vulkanSpecificOptions << CLI_OPT_VULKAN_HLSL_BLOCK_OFFSETS << " ";
    }
    if (buildSettings.m_isUseHlslIoMappingChecked)
    {
        vulkanSpecificOptions << CLI_OPT_VULKAN_HLSL_IOMAP << " ";
    }
    if (buildSettings.m_isEnableValidationLayersChecked)
    {
        cmd << CLI_OPT_VULKAN_VALIDATION << " ";
    }

    if (!buildSettings.m_ICDLocation.empty())
    {
        cmd << CLI_OPT_VULKAN_ICD_LOCATION << " " << "\"" << buildSettings.m_ICDLocation << "\" ";
    }

    if (!buildSettings.m_glslangOptions.empty())
    {
        // Wrap the argument for --glslang-opt with the required token to avoid ambiguity
        // between rga and glslang options.
        cmd << CLI_OPT_VULKAN_GLSLANG_OPTIONS << " " << "\"" << CLI_OPT_GLSLANG_TOKEN <<
            buildSettings.m_glslangOptions << CLI_OPT_GLSLANG_TOKEN << "\" ";
    }

    if (!std::get<CompilerFolderType::Bin>(buildSettings.m_compilerPaths).empty())
    {
        cmd << CLI_OPT_COMPILER_BIN_DIR << " \"" << std::get<CompilerFolderType::Bin>(buildSettings.m_compilerPaths) << "\" ";
    }

    // Append the additional options if required.
    if (additionalOptions && !buildSettings.m_additionalOptions.empty())
    {
        vulkanSpecificOptions << buildSettings.m_additionalOptions;
    }

    // Add the Vulkan-specific options, if there are any.
    if (!vulkanSpecificOptions.str().empty())
    {
        cmd << CLI_OPT_VULKAN_OPTION << " \"" << vulkanSpecificOptions.str() << "\" ";
    }

    str = cmd.str();
    ret = true;

    return ret;
}
