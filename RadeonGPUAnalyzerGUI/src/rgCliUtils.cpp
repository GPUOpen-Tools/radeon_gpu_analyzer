// C++.
#include <sstream>
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgCliUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>

// Common between CLI and GUI.
#include <RadeonGPUAnalyzerGUI/../Utils/include/rgaCliDefs.h>


bool rgCliUtils::GenerateBuildSettingsString(std::shared_ptr<rgCLBuildSettings> pBuildSettings, std::string& str, bool additionalOptions)
{
    bool ret = false;
    assert(pBuildSettings != nullptr);
    if (pBuildSettings != nullptr)
    {
        std::stringstream cmd;

        // Include paths.
        for (std::string& includePath : pBuildSettings->m_additionalIncludeDirectories)
        {
            // Remove trailing slash symbols.
            if (!includePath.empty() && (includePath[includePath.size() - 1] == '\\' || includePath[includePath.size() - 1] == '/'))
            {
                includePath.erase(includePath.size() - 1, 1);
            }
            cmd << CLI_OPT_ADDITIONAL_INCLUDE_PATH << " \"" << includePath << "\" ";
        }

        // Preprocessor directives.
        for (const std::string& currDirective : pBuildSettings->m_predefinedMacros)
        {
            cmd << CLI_OPT_PREPROCESSOR_DIRECTIVE << " " << currDirective << " ";
        }

        // Accumulate the OpenCL-specific options in a separate stream.
        std::stringstream clSpecificOptions;

        // OpenCL-specific build options.
        if (pBuildSettings->m_isAggressiveMathOptimizations)
        {
            clSpecificOptions << CLI_OPT_CL_AGGRESSIVE_OPTIMIZATIONS << " ";
        }
        if (pBuildSettings->m_isCorrectlyRoundDivSqrt)
        {
            clSpecificOptions << CLI_OPT_CL_CORRECT_ROUND_DIV_SQRT << " ";
        }
        if (pBuildSettings->m_isNoNanNorInfinite)
        {
            clSpecificOptions << CLI_OPT_CL_IS_NAN_OR_INIFINITE << " ";
        }
        if (pBuildSettings->m_isUnsafeOptimizations)
        {
            clSpecificOptions << CLI_OPT_CL_UNSAFE_OPTIMIZATIONS << " ";
        }
        if (pBuildSettings->m_isIgnoreZeroSignedness)
        {
            clSpecificOptions << CLI_OPT_CL_IGNORE_ZERO_SIGNEDNESS << " ";
        }
        if (pBuildSettings->m_isEnableMAD)
        {
            clSpecificOptions << CLI_OPT_CL_ENABLE_MAD << " ";
        }
        if (pBuildSettings->m_isStrictAliasing)
        {
            clSpecificOptions << CLI_OPT_CL_STRICT_ALIASING << " ";
        }
        if (pBuildSettings->m_isDenormsAsZeros)
        {
            clSpecificOptions << CLI_OPT_CL_DENORMS_AS_ZEROES << " ";
        }
        if (pBuildSettings->m_isTreatDoubleAsSingle)
        {
            clSpecificOptions << CLI_OPT_CL_TREAT_DOUBLE_AS_SINGLE << " ";
        }

        // Append the additional options if required.
        if (additionalOptions && !pBuildSettings->m_additionalOptions.empty())
        {
            clSpecificOptions << pBuildSettings->m_additionalOptions;
        }

        // Add the OpenCL-specific options, if there are any.
        if (!clSpecificOptions.str().empty())
        {
            cmd << CLI_OPT_CL_OPTION << " \"" << clSpecificOptions.str() << "\" ";
        }

        // Add the alternative compiler paths.
        if (!std::get<CompilerFolderType::Bin>(pBuildSettings->m_compilerPaths).empty())
        {
            cmd << CLI_OPT_COMPILER_BIN_DIR << " \"" << std::get<CompilerFolderType::Bin>(pBuildSettings->m_compilerPaths) << "\" ";
        }
        if (!std::get<CompilerFolderType::Include>(pBuildSettings->m_compilerPaths).empty())
        {
            cmd << CLI_OPT_COMPILER_INC_DIR << " \"" << std::get<CompilerFolderType::Include>(pBuildSettings->m_compilerPaths) << "\" ";
        }
        if (!std::get<CompilerFolderType::Lib>(pBuildSettings->m_compilerPaths).empty())
        {
            cmd << CLI_OPT_COMPILER_LIB_DIR << " \"" << std::get<CompilerFolderType::Lib>(pBuildSettings->m_compilerPaths) << "\" ";
        }

        if (pBuildSettings->m_optimizationLevel != pBuildSettings->OPENCL_DEFAULT_OPT_LEVEL)
        {
            // Tokens to identify the user's selection.
            const char* OPTIMIZATION_LEVEL_0_TOKEN = "O0";
            const char* OPTIMIZATION_LEVEL_1_TOKEN = "O1";
            const char* OPTIMIZATION_LEVEL_2_TOKEN = "O2";
            const char* OPTIMIZATION_LEVEL_3_TOKEN = "O3";

            if (pBuildSettings->m_optimizationLevel.find(OPTIMIZATION_LEVEL_0_TOKEN) != std::string::npos)
            {
                cmd << CLI_OPT_CL_OPTMIZATION_LEVEL_0 << " ";
            }
            else if (pBuildSettings->m_optimizationLevel.find(OPTIMIZATION_LEVEL_1_TOKEN) != std::string::npos)
            {
                cmd << CLI_OPT_CL_OPTMIZATION_LEVEL_1 << " ";
            }
            else if (pBuildSettings->m_optimizationLevel.find(OPTIMIZATION_LEVEL_2_TOKEN) != std::string::npos)
            {
                cmd << CLI_OPT_CL_OPTMIZATION_LEVEL_2 << " ";
            }
            else if (pBuildSettings->m_optimizationLevel.find(OPTIMIZATION_LEVEL_3_TOKEN) != std::string::npos)
            {
                cmd << CLI_OPT_CL_OPTMIZATION_LEVEL_3 << " ";
            }
        }

        str = cmd.str();
        ret = true;
    }

    return ret;
}
