#pragma once

// C++.
#include <map>
#include <memory>
#include <string>
#include <vector>

// Qt.
#include <QMap>
#include <QString>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>

// *** OPENCL STRING CONSTANTS - START ***

// Shader source file extensions.
static const char* STR_SOURCE_FILE_EXTENSION_CL = ".cl";

// OpenCL API Name.
static const char* STR_API_NAME_OPENCL = "OpenCL";
static const char* STR_API_ABBREVIATION_OPENCL = "CL";

// Default OpenCL Build Settings string.
static const char* STR_DEFAULT_BUILD_SETTINGS_OPENCL = "Default OpenCL build settings";

// Create New File menu item.
static const char* STR_MENU_BAR_CREATE_NEW_FILE_OPENCL = "&Create new .cl file";
static const char* STR_MENU_BAR_CREATE_NEW_FILE_TOOLTIP_OPENCL = "Create a new OpenCL (.cl) source file (Ctrl+N).";

// Open Existing File menu item.
static const char* STR_MENU_BAR_OPEN_EXISTING_FILE_OPENCL = "&Open existing .cl file";
static const char* STR_MENU_BAR_OPEN_EXISTING_FILE_TOOLTIP_OPENCL = "Open an existing OpenCL (.cl) file (Ctrl+O).";

// Rename project dialog title string.
static const char* STR_RENAME_PROJECT_DIALOG_BOX_TITLE_OPENCL = "New OpenCL Project";

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

// *** DEFAULT KERNEL SOURCE CODE STRINGS - START ***

static const char* STR_NEW_FILE_TEMPLATE_CODE_OPENCL_A = "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n__kernel void ";
static const char* STR_NEW_FILE_TEMPLATE_CODE_OPENCL_B = "MyKernel()\n{\n}";

// *** DEFAULT KERNEL SOURCE CODE STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** OPENCL TYPE DECLARATIONS - START ***

// OpenCL build settings.
struct rgBuildSettingsOpenCL : public rgBuildSettings
{
    rgBuildSettingsOpenCL() = default;
    virtual ~rgBuildSettingsOpenCL() = default;

    // Copy constructor used to initialize using another instance.
    rgBuildSettingsOpenCL(const rgBuildSettingsOpenCL& other) :
        rgBuildSettings(other),
        m_optimizationLevel(other.m_optimizationLevel),
        m_isTreatDoubleAsSingle(other.m_isTreatDoubleAsSingle),
        m_isDenormsAsZeros(other.m_isDenormsAsZeros),
        m_isStrictAliasing(other.m_isStrictAliasing),
        m_isEnableMAD(other.m_isEnableMAD),
        m_isIgnoreZeroSignedness(other.m_isIgnoreZeroSignedness),
        m_isUnsafeOptimizations(other.m_isUnsafeOptimizations),
        m_isNoNanNorInfinite(other.m_isNoNanNorInfinite),
        m_isAggressiveMathOptimizations(other.m_isAggressiveMathOptimizations),
        m_isCorrectlyRoundDivSqrt(other.m_isCorrectlyRoundDivSqrt) {}

    // Determine if the supplied settings are the same as the current settings.
    virtual bool HasSameSettings(const rgBuildSettingsOpenCL& other) const
    {
        bool isSame = rgBuildSettings::HasSameSettings(other) &&
            m_optimizationLevel == other.m_optimizationLevel &&
            m_isTreatDoubleAsSingle == other.m_isTreatDoubleAsSingle &&
            m_isDenormsAsZeros == other.m_isDenormsAsZeros &&
            m_isStrictAliasing == other.m_isStrictAliasing &&
            m_isEnableMAD == other.m_isEnableMAD &&
            m_isIgnoreZeroSignedness == other.m_isIgnoreZeroSignedness &&
            m_isUnsafeOptimizations == other.m_isUnsafeOptimizations &&
            m_isNoNanNorInfinite == other.m_isNoNanNorInfinite &&
            m_isAggressiveMathOptimizations == other.m_isAggressiveMathOptimizations &&
            m_isCorrectlyRoundDivSqrt == other.m_isCorrectlyRoundDivSqrt;

        return isSame;
    }

    // Default values for specific settings.
    std::string OPENCL_DEFAULT_OPT_LEVEL = "Default";

    // OpenCL-specific build settings.
    std::string m_optimizationLevel = OPENCL_DEFAULT_OPT_LEVEL;
    bool m_isTreatDoubleAsSingle = false;
    bool m_isDenormsAsZeros = false;
    bool m_isStrictAliasing = false;
    bool m_isEnableMAD = false;
    bool m_isIgnoreZeroSignedness = false;
    bool m_isUnsafeOptimizations = false;
    bool m_isNoNanNorInfinite = false;
    bool m_isAggressiveMathOptimizations = false;
    bool m_isCorrectlyRoundDivSqrt = false;
};

// A clone of an OpenCL project.
struct rgProjectCloneOpenCL : public rgProjectClone
{
    rgProjectCloneOpenCL()
    {
        // Instantiate an OpenCL build settings instance by default.
        m_pBuildSettings = std::make_shared<rgBuildSettingsOpenCL>();
    }

    rgProjectCloneOpenCL(const std::string& cloneName, std::shared_ptr<rgBuildSettingsOpenCL> pBuildSettings) :
        rgProjectClone(cloneName, pBuildSettings){}
};

// An OpenCL project.
struct rgProjectOpenCL : public rgProject
{
    rgProjectOpenCL() : rgProject("", "", rgProjectAPI::OpenCL) {}

    // CTOR #1.
    rgProjectOpenCL(const std::string& projectName, const std::string& projectFileFullPath) : rgProject(projectName,
        projectFileFullPath, rgProjectAPI::OpenCL) {}

    // CTOR #2.
    rgProjectOpenCL(const std::string& projectName, const std::string& projectFileFullPath,
        const std::vector<std::shared_ptr<rgProjectClone>>& clones) :
        rgProject(projectName, projectFileFullPath, rgProjectAPI::OpenCL, clones) {}
};

// *** OPENCL TYPE DECLARATIONS - END ***