// C++.
#include <sstream>
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/rg_cli_utils.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"

// Common between CLI and GUI.
#include "source/common/rga_cli_defs.h"

bool RgCliUtils::GenerateOpenclBuildSettingsString(const RgBuildSettingsOpencl& build_settings, std::string& str, bool additional_options)
{
    bool ret = false;
    std::stringstream cmd;

    // Include paths.
    for (std::string include_path : build_settings.additional_include_directories)
    {
        // Remove trailing slash symbols.
        if (!include_path.empty() && (include_path[include_path.size() - 1] == '\\' || include_path[include_path.size() - 1] == '/'))
        {
            include_path.erase(include_path.size() - 1, 1);
        }
        cmd << kStrCliOptAdditionalIncludePath << " \"" << include_path << "\" ";
    }

    // Preprocessor directives.
    for (const std::string& curr_directive : build_settings.predefined_macros)
    {
        cmd << kStrCliOptPreprocessorDirective << " " << curr_directive << " ";
    }

    // Accumulate the OpenCL-specific options in a separate stream.
    std::stringstream cl_specific_options;

    // OpenCL-specific build options.
    if (build_settings.is_aggressive_math_optimizations_)
    {
        cl_specific_options << kStrCliOptClAggressiveOptimizations << " ";
    }
    if (build_settings.is_correctly_round_div_sqrt_)
    {
        cl_specific_options << kStrCliOptClCorrectRoundDivSqrt << " ";
    }
    if (build_settings.is_no_nan_nor_infinite_)
    {
        cl_specific_options << kStrCliOptClIsNanOrInfinite << " ";
    }
    if (build_settings.is_unsafe_optimizations_)
    {
        cl_specific_options << kStrCliOptClUnsafeOptimizations << " ";
    }
    if (build_settings.is_ignore_zero_signedness_)
    {
        cl_specific_options << kStrCliOptClIgnoreZeroSignedness << " ";
    }
    if (build_settings.is_enable_mad_)
    {
        cl_specific_options << kStrCliOptClEnableMad << " ";
    }
    if (build_settings.is_strict_aliasing_)
    {
        cl_specific_options << kStrCliOptClStrictAliasing << " ";
    }
    if (build_settings.is_denorms_as_zeros_)
    {
        cl_specific_options << kStrCliOptClDenormsAsZeroes << " ";
    }
    if (build_settings.is_treat_double_as_single_)
    {
        cl_specific_options << kStrCliOptClTreatDoubleAsSingle << " ";
    }

    // Append the additional options if required.
    if (additional_options && !build_settings.additional_options.empty())
    {
        cl_specific_options << build_settings.additional_options;
    }

    // Add the OpenCL-specific options, if there are any.
    if (!cl_specific_options.str().empty())
    {
        cmd << kStrCliOptClOption << " \"" << cl_specific_options.str() << "\" ";
    }

    // Add the alternative compiler paths.
    if (!std::get<CompilerFolderType::kBin>(build_settings.compiler_paths).empty())
    {
        cmd << kStrCliOptCompilerBinDir << " \"" << std::get<CompilerFolderType::kBin>(build_settings.compiler_paths) << "\" ";
    }
    if (!std::get<CompilerFolderType::kInclude>(build_settings.compiler_paths).empty())
    {
        cmd << kStrCliOptCompilerIncDir << " \"" << std::get<CompilerFolderType::kInclude>(build_settings.compiler_paths) << "\" ";
    }
    if (!std::get<CompilerFolderType::kLib>(build_settings.compiler_paths).empty())
    {
        cmd << kStrCliOptCompilerLibDir << " \"" << std::get<CompilerFolderType::kLib>(build_settings.compiler_paths) << "\" ";
    }

    if (build_settings.optimization_level_ != build_settings.kOpenclDefaultOptLevel)
    {
        // Tokens to identify the user's selection.
        const char* kOptimizationLevel0Token = "O0";
        const char* kOptimizationLevel1Token = "O1";
        const char* kOptimizationLevel2Token = "O2";
        const char* kOptimizationLevel3Token = "O3";

        if (build_settings.optimization_level_.find(kOptimizationLevel0Token) != std::string::npos)
        {
            cmd << kStrCliOptClOptimizationLevel0 << " ";
        }
        else if (build_settings.optimization_level_.find(kOptimizationLevel1Token) != std::string::npos)
        {
            cmd << kStrCliOptClOptimizationLevel1 << " ";
        }
        else if (build_settings.optimization_level_.find(kOptimizationLevel2Token) != std::string::npos)
        {
            cmd << kStrCliOptClOptimizationLevel2 << " ";
        }
        else if (build_settings.optimization_level_.find(kOptimizationLevel3Token) != std::string::npos)
        {
            cmd << kStrCliOptClOptimizationLevel3 << " ";
        }
    }

    str = cmd.str();
    ret = true;

    return ret;
}

bool RgCliUtils::GenerateVulkanBuildSettingsString(const RgBuildSettingsVulkan& build_settings, std::string& str, bool additional_options)
{
    bool ret = false;
    std::stringstream cmd;

    // Include paths.
    for (std::string include_path : build_settings.additional_include_directories)
    {
        // Remove trailing slash symbols.
        if (!include_path.empty() && (include_path[include_path.size() - 1] == '\\' || include_path[include_path.size() - 1] == '/'))
        {
            include_path.erase(include_path.size() - 1, 1);
        }
        cmd << kStrCliOptAdditionalIncludePath << " \"" << include_path << "\" ";
    }

    // Preprocessor directives.
    for (const std::string& curr_directive : build_settings.predefined_macros)
    {
        cmd << kStrCliOptPreprocessorDirective << " " << curr_directive << " ";
    }

    // Accumulate the Vulkan-specific options in a separate stream.
    std::stringstream vulkan_specific_options;

    // Vulkan-specific build options.
    if (build_settings.is_generate_debug_info_checked)
    {
        vulkan_specific_options << kStrCliOptVulkanGenerateDebugInformation << " ";
    }
    if (build_settings.is_no_explicit_bindings_checked)
    {
        vulkan_specific_options << kStrCliOptVulkanNoExplicitBindings << " ";
    }
    if (build_settings.is_use_hlsl_block_offsets_checked)
    {
        vulkan_specific_options << kStrCliOptVulkanHlslBlockOffsets << " ";
    }
    if (build_settings.is_use_hlsl_io_mapping_checked)
    {
        vulkan_specific_options << kStrCliOptVulkanHlslIomap << " ";
    }
    if (build_settings.is_enable_validation_layers_checked)
    {
        cmd << kStrCliOptVulkanValidation << " ";
    }

    if (!build_settings.icd_location.empty())
    {
        cmd << kStrCliOptVulkanIcdLocation << " " << "\"" << build_settings.icd_location << "\" ";
    }

    if (!build_settings.glslang_options.empty())
    {
        // Wrap the argument for --glslang-opt with the required token to avoid ambiguity
        // between rga and glslang options.
        cmd << kStrCliOptVulkanGlslangOptions << " " << "\"" << kStrCliOptGlslangToken <<
            build_settings.glslang_options << kStrCliOptGlslangToken << "\" ";
    }

    if (!std::get<CompilerFolderType::kBin>(build_settings.compiler_paths).empty())
    {
        cmd << kStrCliOptCompilerBinDir << " \"" << std::get<CompilerFolderType::kBin>(build_settings.compiler_paths) << "\" ";
    }

    // Append the additional options if required.
    if (additional_options && !build_settings.additional_options.empty())
    {
        vulkan_specific_options << build_settings.additional_options;
    }

    // Add the Vulkan-specific options, if there are any.
    if (!vulkan_specific_options.str().empty())
    {
        cmd << kStrCliOptVulkanOption << " \"" << vulkan_specific_options.str() << "\" ";
    }

    str = cmd.str();
    ret = true;

    return ret;
}
