//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for the RGA gui OpenCL data types.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DATA_TYPES_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DATA_TYPES_OPENCL_H_

// C++.
#include <map>
#include <memory>
#include <string>
#include <vector>

// Qt.
#include <QMap>
#include <QString>

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"

// *** OPENCL STRING CONSTANTS - START ***

// Shader source file extensions.
static const char* kStrSourceFileExtensionCl = ".cl";

// OpenCL API Name.
static const char* kStrApiNameOpencl = "OpenCL";
static const char* kStrApiAbbreviationOpencl = "CL";

// Default OpenCL Build Settings string.
static const char* kStrDefaultBuildSettingsOpencl = "Default OpenCL build settings";

// Create New File menu item.
static const char* kStrMenuBarCreateNewFileOpencl = "&Create new .cl file";
static const char* kStrMenuBarCreateNewFileTooltipOpencl = "Create a new OpenCL (.cl) source file (Ctrl+N).";

// Open Existing File menu item.
static const char* kStrMenuBarOpenExistingFileOpencl = "&Open existing .cl file";
static const char* kStrMenuBarOpenExistingFileTooltipOpencl = "Open an existing OpenCL (.cl) file (Ctrl+O).";

// Rename project dialog title string.
static const char* kStrRenameProjectDialogBoxTitleOpencl = "New OpenCL Project";

// Check box tool tip lookup map.
static const QMap<QString, QString> TOOLTIPS =
{
    { "doubleAsSingleCheckBox",                "<html><head/><body><p>Treat double precision floating-point constant as single precision constant.</p></body></html>" },
    { "flushDenormalizedCheckBox",             "<html><head/><body><p>This option controls how single precision and double precision denormalized numbers are handled. If specified as a build option, the single precision denormalized numbers may be flushed to zero and if the optional extension for double precision is supported, double precision denormalized numbers may also be flushed to zero. This is intended to be a performance hint and the OpenCL compiler can choose not to flush denorms to zero if the device supports single precision (or double precision) denormalized numbers.</p><p>This option is ignored for single precision numbers if the device does not support single precision denormalized numbers i.e. CL_FP_DENORM bit is not set in CL_DEVICE_SINGLE_FP_CONFIG.</p><p>This option is ignored for double precision numbers if the device does not support double precision or if it does support double precison but CL_FP_DENORM bit is not set in CL_DEVICE_DOUBLE_FP_CONFIG.</p><p>This flag only applies for scalar and vector single precision floating-point variables and computations on these floating-point variables inside a program. It does not apply to reading from or writing to image objects.</p></body></html>" },
    { "strictAliasingCheckBox",                "<html><head/><body><p>Allow the compiler to assume the most strict aliasing rules.</p></body></html>"},
    { "enableMADCheckBox",                     "<html><head/><body><p>Allow a * b + c to be replaced by a mad. The mad computes a * b + c with reduced accuracy. For example, some OpenCL devices implement mad as truncate the result of a * b before adding it to c.</p></body></html>"},
    { "ignoreZeroSignednessCheckBox",          "<html><head/><body><p>Allow optimizations for floating-point arithmetic that ignore the signedness of zero. IEEE 754 arithmetic specifies the behavior of distinct +0.0 and -0.0 values, which then prohibits simplification of expressions such as x+0.0 or 0.0*x (even with -clfinite-math only). This option implies that the sign of a zero result isn't significant.</p></body></html>"},
    { "allowUnsafeOptimizationsCheckBox",      "<html><head/><body><p>Allow optimizations for floating-point arithmetic that (a) assume that arguments and results are valid, (b) may violate IEEE 754 standard and (c) may violate the OpenCL numerical compliance requirements as defined in section 7.4 for single-precision floating-point, section 9.3.9 for double-precision floating-point, and edge case behavior in section 7.5. This option includes the -cl-no-signed-zeros and -cl-mad-enable options.</p></body></html>"},
    { "assumeNoNanNorInfiniteCheckBox",        "<html><head/><body><p>Allow optimizations for floating-point arithmetic that assume that arguments and results are not NaNs or +/-?. This option may violate the OpenCL numerical compliance requirements defined in in section 7.4 for single-precision floating-point, section 9.3.9 for double-precision floating-point, and edge case behavior in section 7.5.</p></body></html>"},
    { "aggressiveMathOptimizationsCheckBox",   "<html><head/><body><p>Sets the optimization options -cl-finite-math-only and -cl-unsafe-math-optimizations. This allows optimizations for floating-point arithmetic that may violate the IEEE 754 standard and the OpenCL numerical compliance requirements defined in the specification in section 7.4 for single-precision floating-point, section 9.3.9 for double-precision floating-point, and edge case behavior in section 7.5. This option causes the preprocessor macro __FAST_RELAXED_MATH__ to be defined in the OpenCL program.</p></body></html>"},
    { "correctlyRoundSinglePrecisionCheckBox", "<html><head/><body><p>Specifies that single precision floating-point divide (x/y and 1/x) and sqrt used in the program source are correctly rounded. If this option is not specified, the minimum numerical accuracy of single precision floating-point divide and sqrt are as defined in section 7.4 of the OpenCL specification. This build option can only be specified if the CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT is set in CL_DEVICE_SINGLE_FP_CONFIG (as defined in in the table of allowed values for param_name for clGetDeviceInfo) for devices that the program is being build. clBuildProgram or clCompileProgram will fail to compile the program for a device if the -cl-fp32-correctly-rounded-divide-sqrt option is specified and CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT is not set for the device.</p></body></html>"} };

// *** OPENCL STRING CONSTANTS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** DEFAULT KERNEL SOURCE CODE  - START ***

static const char* kStrNewFileTemplateCodeOpenclA = "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n__kernel void ";
static const char* kStrNewFileTemplateCodeOpenclB = "MyKernel()\n{\n}";

// *** DEFAULT KERNEL SOURCE CODE  - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** OPENCL TYPE DECLARATIONS - START ***

// OpenCL build settings.
struct RgBuildSettingsOpencl : public RgBuildSettings
{
    RgBuildSettingsOpencl() = default;
    virtual ~RgBuildSettingsOpencl() = default;

    // Copy constructor used to initialize using another instance.
    RgBuildSettingsOpencl(const RgBuildSettingsOpencl& other) :
        RgBuildSettings(other),
        optimization_level_(other.optimization_level_),
        is_treat_double_as_single_(other.is_treat_double_as_single_),
        is_denorms_as_zeros_(other.is_denorms_as_zeros_),
        is_strict_aliasing_(other.is_strict_aliasing_),
        is_enable_mad_(other.is_enable_mad_),
        is_ignore_zero_signedness_(other.is_ignore_zero_signedness_),
        is_unsafe_optimizations_(other.is_unsafe_optimizations_),
        is_no_nan_nor_infinite_(other.is_no_nan_nor_infinite_),
        is_aggressive_math_optimizations_(other.is_aggressive_math_optimizations_),
        is_correctly_round_div_sqrt_(other.is_correctly_round_div_sqrt_) {}

    // Determine if the supplied settings are the same as the current settings.
    virtual bool HasSameSettings(const RgBuildSettingsOpencl& other) const
    {
        bool isSame = RgBuildSettings::HasSameSettings(other) &&
            optimization_level_ == other.optimization_level_ &&
            is_treat_double_as_single_ == other.is_treat_double_as_single_ &&
            is_denorms_as_zeros_ == other.is_denorms_as_zeros_ &&
            is_strict_aliasing_ == other.is_strict_aliasing_ &&
            is_enable_mad_ == other.is_enable_mad_ &&
            is_ignore_zero_signedness_ == other.is_ignore_zero_signedness_ &&
            is_unsafe_optimizations_ == other.is_unsafe_optimizations_ &&
            is_no_nan_nor_infinite_ == other.is_no_nan_nor_infinite_ &&
            is_aggressive_math_optimizations_ == other.is_aggressive_math_optimizations_ &&
            is_correctly_round_div_sqrt_ == other.is_correctly_round_div_sqrt_;

        return isSame;
    }

    // Default values for specific settings.
    std::string kOpenclDefaultOptLevel = "Default";

    // OpenCL-specific build settings.
    std::string optimization_level_ = kOpenclDefaultOptLevel;
    bool is_treat_double_as_single_ = false;
    bool is_denorms_as_zeros_ = false;
    bool is_strict_aliasing_ = false;
    bool is_enable_mad_ = false;
    bool is_ignore_zero_signedness_ = false;
    bool is_unsafe_optimizations_ = false;
    bool is_no_nan_nor_infinite_ = false;
    bool is_aggressive_math_optimizations_ = false;
    bool is_correctly_round_div_sqrt_ = false;
};

// A clone of an OpenCL project.
struct RgProjectCloneOpencl : public RgProjectClone
{
    RgProjectCloneOpencl()
    {
        // Instantiate an OpenCL build settings instance by default.
        build_settings = std::make_shared<RgBuildSettingsOpencl>();
    }

    RgProjectCloneOpencl(const std::string& clone_name, std::shared_ptr<RgBuildSettingsOpencl> build_settings) :
        RgProjectClone(clone_name, build_settings){}
};

// An OpenCL project.
struct RgProjectOpencl : public RgProject
{
    RgProjectOpencl() : RgProject("", "", RgProjectAPI::kOpenCL) {}

    // CTOR #1.
    RgProjectOpencl(const std::string& project_name, const std::string& project_file_full_path) : RgProject(project_name,
        project_file_full_path, RgProjectAPI::kOpenCL) {}

    // CTOR #2.
    RgProjectOpencl(const std::string& project_name, const std::string& project_file_full_path,
        const std::vector<std::shared_ptr<RgProjectClone>>& clones) :
        RgProject(project_name, project_file_full_path, RgProjectAPI::kOpenCL, clones) {}
};

// *** OPENCL TYPE DECLARATIONS - END ***
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DATA_TYPES_OPENCL_H_
