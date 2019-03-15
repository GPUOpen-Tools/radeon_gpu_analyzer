#pragma once

// C++.
#include <map>
#include <memory>
#include <string>
#include <vector>

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