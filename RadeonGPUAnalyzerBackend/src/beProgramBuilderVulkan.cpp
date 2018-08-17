//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <sstream>

// Infra.
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Local.
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderVulkan.h>
#include <RadeonGPUAnalyzerBackend/include/beInclude.h>
#include <RadeonGPUAnalyzerBackend/include/beUtils.h>
#include <RadeonGPUAnalyzerBackend/include/beStringConstants.h>
#include <DeviceInfoUtils.h>

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

static const std::string  STR_VERT_SPV_OUTPUT_FILE_NAME = "vert.spv";
static const std::string  STR_TESC_SPV_OUTPUT_FILE_NAME = "tesc.spv";
static const std::string  STR_TESE_SPV_OUTPUT_FILE_NAME = "tese.spv";
static const std::string  STR_GEOM_SPV_OUTPUT_FILE_NAME = "geom.spv";
static const std::string  STR_FRAG_SPV_OUTPUT_FILE_NAME = "frag.spv";
static const std::string  STR_COMP_SPV_OUTPUT_FILE_NAME = "comp.spv";

static const std::string  STR_VERT_PALIL_OUTPUT_FILE_NAME = "vert.palIl";
static const std::string  STR_TESC_PALIL_OUTPUT_FILE_NAME = "tesc.palIl";
static const std::string  STR_TESE_PALIL_OUTPUT_FILE_NAME = "tese.palIl";
static const std::string  STR_GEOM_PALIL_OUTPUT_FILE_NAME = "geom.palIl";
static const std::string  STR_FRAG_PALIL_OUTPUT_FILE_NAME = "frag.palIl";
static const std::string  STR_COMP_PALIL_OUTPUT_FILE_NAME = "comp.palIl";

static const std::string  STR_AMDSPV_DEVICE_GFX900 = "900";
static const std::string  STR_AMDSPV_DEVICE_GFX902 = "902";

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

// Internally-linked utilities.
static bool GetAmdspvPath(std::string& amdspvPath)
{
#ifdef __linux
    amdspvPath = "amdspv";
#elif _WIN64
    amdspvPath = "x64\\amdspv.exe";
#elif _WIN32
    amdspvPath = "x86\\amdspv.exe";
#endif
    return true;
}

static bool GetGfxIpForVulkan(AMDTDeviceInfoUtils* pDeviceInfo, const VulkanOptions& vulkanOptions, std::string& gfxIpStr)
{
    bool ret = false;
    gfxIpStr.clear();

    if (vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_KALINDI) == 0 ||
        vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_GODAVARI) == 0)
    {
        // Special case #1: 7.x devices.
        gfxIpStr = "7.x";
        ret = true;
    }
    else if (vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_STONEY) == 0 ||
        vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_AMUR) == 0 ||
        vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_NOLAN) == 0)
    {
        // Special case #2: 8.1 devices.
        gfxIpStr = "8.1";
        ret = true;
    }
    else if (vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_GFX900) == 0 ||
             vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_GFX902) == 0)
    {
        // Special case #3: gfx9 devices.
        gfxIpStr =
            vulkanOptions.m_targetDeviceName == DEVICE_NAME_GFX900 ? STR_AMDSPV_DEVICE_GFX900 :
            vulkanOptions.m_targetDeviceName == DEVICE_NAME_GFX902 ? STR_AMDSPV_DEVICE_GFX902 :
            "";
        ret = !gfxIpStr.empty();
    }
    else
    {
        // The standard case.
        size_t deviceGfxIp = 0;
        GDT_HW_GENERATION hwGeneration;
        bool isDeviceHwGenExtracted = pDeviceInfo->GetHardwareGeneration(vulkanOptions.m_targetDeviceName.c_str(), hwGeneration) &&
            beUtils::GdtHwGenToNumericValue(hwGeneration, deviceGfxIp);

        if (isDeviceHwGenExtracted && deviceGfxIp > 0)
        {
            gfxIpStr = std::to_string(deviceGfxIp);
            ret = true;
        }
    }

    return ret;
}

// An internal auxiliary function that returns the correct input prefix for the backend invocation,
// according to the input type. If the input type is GLSL, it simply returns the given GLSL prefix.
// Otherwise, it returns the relevant, fixed, prefix.
static std::string GetInputPrefix(const VulkanOptions& vulkanOptions, const std::string& glslPrefix)
{
    const char* SPIRV_BIN_INPUT_PREFIX = "in.spv=\"";
    const char* SPIRV_TXT_INPUT_PREFIX = "in.spvText=\"";

    std::string ret;
    if (vulkanOptions.m_SourceLanguage == beKA::SourceLanguage_GLSL_Vulkan)
    {
        ret = glslPrefix;
    }
    else if (vulkanOptions.m_SourceLanguage == beKA::SourceLanguage_SPIRV_Vulkan)
    {
        ret = SPIRV_BIN_INPUT_PREFIX;
    }
    else if (vulkanOptions.m_SourceLanguage == beKA::SourceLanguage_SPIRVTXT_Vulkan)
    {
        ret = SPIRV_TXT_INPUT_PREFIX;
    }
    return ret;
}

static beKA::beStatus  AddInputFileNames(const VulkanOptions& options, std::stringstream& cmd)
{
    beKA::beStatus  status = beKA::beStatus_SUCCESS;

    // Indicates that a stage-less input file name was provided.
    bool  isNonStageInput = false;

    // Indicates that some of stage-specific file names was provided (--frag, --vert, etc.).
    bool isStageInput = false;

    if (options.m_SourceLanguage == beKA::SourceLanguage_SPIRV_Vulkan || options.m_SourceLanguage == beKA::SourceLanguage_SPIRVTXT_Vulkan)
    {
        if (!options.m_stagelessInputFile.empty())
        {
            cmd << GetInputPrefix(options, "") << options.m_stagelessInputFile << "\" ";
            isNonStageInput = true;
        }
    }

    // You cannot mix compute and non-compute shaders in Vulkan,
    // so this has to be mutually exclusive.
    if (options.m_pipelineShaders.m_computeShader.isEmpty())
    {
        // Vertex shader.
        if (!options.m_pipelineShaders.m_vertexShader.isEmpty())
        {
            cmd << GetInputPrefix(options, "in.vert.glsl=\"") << options.m_pipelineShaders.m_vertexShader.asASCIICharArray() << "\" ";
            isStageInput = true;
        }

        // Tessellation control shader.
        if (!options.m_pipelineShaders.m_tessControlShader.isEmpty())
        {
            cmd << GetInputPrefix(options, "in.tesc.glsl=\"") << options.m_pipelineShaders.m_tessControlShader.asASCIICharArray() << "\" ";
            isStageInput = true;
        }

        // Tessellation evaluation shader.
        if (!options.m_pipelineShaders.m_tessEvaluationShader.isEmpty())
        {
            cmd << GetInputPrefix(options, "in.tese.glsl=\"") << options.m_pipelineShaders.m_tessEvaluationShader.asASCIICharArray() << "\" ";
            isStageInput = true;
        }

        // Geometry shader.
        if (!options.m_pipelineShaders.m_geometryShader.isEmpty())
        {
            cmd << GetInputPrefix(options, "in.geom.glsl=\"") << options.m_pipelineShaders.m_geometryShader.asASCIICharArray() << "\" ";
            isStageInput = true;
        }

        // Fragment shader.
        if (!options.m_pipelineShaders.m_fragmentShader.isEmpty())
        {
            cmd << GetInputPrefix(options, "in.frag.glsl=\"") << options.m_pipelineShaders.m_fragmentShader.asASCIICharArray() << "\" ";
            isStageInput = true;
        }
    }
    else
    {
        // Compute shader.
        cmd << GetInputPrefix(options, "in.comp.glsl=\"") << options.m_pipelineShaders.m_computeShader.asASCIICharArray() << "\" ";
        isStageInput = true;
    }

    if (!isNonStageInput && !isStageInput)
    {
        status = beKA::beStatus_VulkanNoInputFile;
    }
    else if (isNonStageInput && isStageInput)
    {
        status = beKA::beStatus_VulkanMixedInputFiles;
    }

    return status;
}

static void AddOutputFileNames(const VulkanOptions& options, std::stringstream& cmd)
{
    bool isSpv = (options.m_SourceLanguage == beKA::SourceLanguage_SPIRV_Vulkan ||
                  options.m_SourceLanguage == beKA::SourceLanguage_SPIRVTXT_Vulkan);

    auto  AddOutputFile = [&](bool flag, const std::string& option, const std::string& fileName)
    {
        if (flag || isSpv)
        {
            cmd << option << "\"" << fileName << "\"" << " ";
        }
    };

    // SPIR-V binaries generation.
    if (options.m_isSpirvBinariesRequired)
    {
        // Compute.
        AddOutputFile(!options.m_pipelineShaders.m_computeShader.isEmpty(), "out.comp.spv=", STR_COMP_SPV_OUTPUT_FILE_NAME);

        if (options.m_pipelineShaders.m_computeShader.isEmpty() || isSpv)
        {
            // Vertex.
            AddOutputFile(!options.m_pipelineShaders.m_vertexShader.isEmpty(), "out.vert.spv=", STR_VERT_SPV_OUTPUT_FILE_NAME);
            // Tessellation control.
            AddOutputFile(!options.m_pipelineShaders.m_tessControlShader.isEmpty(), "out.tesc.spv=", STR_TESC_SPV_OUTPUT_FILE_NAME);
            // Tessellation evaluation.
            AddOutputFile(!options.m_pipelineShaders.m_tessEvaluationShader.isEmpty(), "out.tese.spv=", STR_TESE_SPV_OUTPUT_FILE_NAME);
            // Geometry.
            AddOutputFile(!options.m_pipelineShaders.m_geometryShader.isEmpty(), "out.geom.spv=", STR_GEOM_SPV_OUTPUT_FILE_NAME);
            // Fragment.
            AddOutputFile(!options.m_pipelineShaders.m_fragmentShader.isEmpty(), "out.frag.spv=", STR_FRAG_SPV_OUTPUT_FILE_NAME);
        }
    }

    // AMD IL Binaries generation (for now we only support PAL IL).
    if (options.m_isAmdPalIlBinariesRequired)
    {
        // Compute.
        AddOutputFile(!options.m_pipelineShaders.m_computeShader.isEmpty(), "out.comp.palIl=", STR_COMP_PALIL_OUTPUT_FILE_NAME);

        if (options.m_pipelineShaders.m_computeShader.isEmpty() || isSpv)
        {
            // Vertex.
            AddOutputFile(!options.m_pipelineShaders.m_vertexShader.isEmpty(), "out.vert.palIl=", STR_VERT_PALIL_OUTPUT_FILE_NAME);
            // Tessellation control.
            AddOutputFile(!options.m_pipelineShaders.m_tessControlShader.isEmpty(), "out.tesc.palIl=", STR_TESC_PALIL_OUTPUT_FILE_NAME);
            // Tessellation evaluation.
            AddOutputFile(!options.m_pipelineShaders.m_tessEvaluationShader.isEmpty(), "out.tese.palIl=", STR_TESE_PALIL_OUTPUT_FILE_NAME);
            // Geometry.
            AddOutputFile(!options.m_pipelineShaders.m_geometryShader.isEmpty(), "out.geom.palIl=", STR_GEOM_PALIL_OUTPUT_FILE_NAME);
            // Fragment.
            AddOutputFile(!options.m_pipelineShaders.m_fragmentShader.isEmpty(), "out.frag.palIl=", STR_FRAG_PALIL_OUTPUT_FILE_NAME);
        }
    }

    // AMD IL disassembly generation (for now we only support PAL IL).
    if (options.m_isAmdPalIlDisassemblyRequired)
    {
        // Compute.
        AddOutputFile(!options.m_pipelineShaders.m_computeShader.isEmpty(), "out.comp.palIlText=", options.m_pailIlDisassemblyOutputFiles.m_computeShader.asASCIICharArray());

        if (options.m_pipelineShaders.m_computeShader.isEmpty() || isSpv)
        {
            // Vertex.
            AddOutputFile(!options.m_pipelineShaders.m_vertexShader.isEmpty(), "out.vert.palIlText=", options.m_pailIlDisassemblyOutputFiles.m_vertexShader.asASCIICharArray());
            // Tessellation control.
            AddOutputFile(!options.m_pipelineShaders.m_tessControlShader.isEmpty(), "out.tesc.palIlText=", options.m_pailIlDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray());
            // Tessellation evaluation.
            AddOutputFile(!options.m_pipelineShaders.m_tessEvaluationShader.isEmpty(), "out.tese.palIlText=", options.m_pailIlDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray());
            // Geometry.
            AddOutputFile(!options.m_pipelineShaders.m_geometryShader.isEmpty(), "out.geom.palIlText=", options.m_pailIlDisassemblyOutputFiles.m_geometryShader.asASCIICharArray());
            // Fragment.
            AddOutputFile(!options.m_pipelineShaders.m_fragmentShader.isEmpty(), "out.frag.palIlText=", options.m_pailIlDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray());
        }
    }

    // AMD ISA binary generation.
    if (options.m_isAmdIsaBinariesRequired)
    {
        // Compute.
        AddOutputFile(!options.m_pipelineShaders.m_computeShader.isEmpty(), "out.comp.isa=", options.m_isaBinaryFiles.m_computeShader.asASCIICharArray());

        if (options.m_pipelineShaders.m_computeShader.isEmpty() || isSpv)
        {
            // Vertex.
            AddOutputFile(!options.m_pipelineShaders.m_vertexShader.isEmpty(), "out.vert.isa=", options.m_isaBinaryFiles.m_vertexShader.asASCIICharArray());
            // Tessellation control.
            AddOutputFile(!options.m_pipelineShaders.m_tessControlShader.isEmpty(), "out.tesc.isa=", options.m_isaBinaryFiles.m_tessControlShader.asASCIICharArray());
            // Tessellation evaluation.
            AddOutputFile(!options.m_pipelineShaders.m_tessEvaluationShader.isEmpty(), "out.tese.isa=", options.m_isaBinaryFiles.m_tessEvaluationShader.asASCIICharArray());
            // Geometry.
            AddOutputFile(!options.m_pipelineShaders.m_geometryShader.isEmpty(), "out.geom.isa=", options.m_isaBinaryFiles.m_geometryShader.asASCIICharArray());
            // Fragment.
            AddOutputFile(!options.m_pipelineShaders.m_fragmentShader.isEmpty(), "out.frag.isa=", options.m_isaBinaryFiles.m_fragmentShader.asASCIICharArray());
        }
    }

    // AMD ISA disassembly generation.
    if (options.m_isAmdIsaDisassemblyRequired)
    {
        // Compute.
        AddOutputFile(!options.m_pipelineShaders.m_computeShader.isEmpty(), "out.comp.isaText=", options.m_isaDisassemblyOutputFiles.m_computeShader.asASCIICharArray());

        if (options.m_pipelineShaders.m_computeShader.isEmpty() || isSpv)
        {
            // Vertex.
            AddOutputFile(!options.m_pipelineShaders.m_vertexShader.isEmpty(), "out.vert.isaText=", options.m_isaDisassemblyOutputFiles.m_vertexShader.asASCIICharArray());
            // Tessellation control.
            AddOutputFile(!options.m_pipelineShaders.m_tessControlShader.isEmpty(), "out.tesc.isaText=", options.m_isaDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray());
            // Tessellation evaluation.
            AddOutputFile(!options.m_pipelineShaders.m_tessEvaluationShader.isEmpty(), "out.tese.isaText=", options.m_isaDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray());
            // Geometry.
            AddOutputFile(!options.m_pipelineShaders.m_geometryShader.isEmpty(), "out.geom.isaText=", options.m_isaDisassemblyOutputFiles.m_geometryShader.asASCIICharArray());
            // Fragment.
            AddOutputFile(!options.m_pipelineShaders.m_fragmentShader.isEmpty(), "out.frag.isaText=", options.m_isaDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray());
        }
    }

    // Shader compiler statistics disassembly generation.
    if (options.m_isScStatsRequired)
    {
        // Compute.
        AddOutputFile(!options.m_pipelineShaders.m_computeShader.isEmpty(), "out.comp.isaInfo=", options.m_scStatisticsOutputFiles.m_computeShader.asASCIICharArray());

        if (options.m_pipelineShaders.m_computeShader.isEmpty() || isSpv)
        {
            // Vertex.
            AddOutputFile(!options.m_pipelineShaders.m_vertexShader.isEmpty(), "out.vert.isaInfo=", options.m_scStatisticsOutputFiles.m_vertexShader.asASCIICharArray());
            // Tessellation control.
            AddOutputFile(!options.m_pipelineShaders.m_tessControlShader.isEmpty(), "out.tesc.isaInfo=", options.m_scStatisticsOutputFiles.m_tessControlShader.asASCIICharArray());
            // Tessellation evaluation.
            AddOutputFile(!options.m_pipelineShaders.m_tessEvaluationShader.isEmpty(), "out.tese.isaInfo=", options.m_scStatisticsOutputFiles.m_tessEvaluationShader.asASCIICharArray());
            // Geometry.
            AddOutputFile(!options.m_pipelineShaders.m_geometryShader.isEmpty(), "out.geom.isaInfo=", options.m_scStatisticsOutputFiles.m_geometryShader.asASCIICharArray());
            // Fragment.
            AddOutputFile(!options.m_pipelineShaders.m_fragmentShader.isEmpty(), "out.frag.isaInfo=", options.m_scStatisticsOutputFiles.m_fragmentShader.asASCIICharArray());
        }
    }
}

beProgramBuilderVulkan::beProgramBuilderVulkan()
{
}

beProgramBuilderVulkan::~beProgramBuilderVulkan()
{
}

beKA::beStatus beProgramBuilderVulkan::GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(binopts);
    GT_UNREFERENCED_PARAMETER(binary);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderVulkan::GetKernelILText(const std::string& device, const std::string& kernel, std::string& il)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(il);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderVulkan::GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(isa);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderVulkan::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(analysis);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderVulkan::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    (void)table;
    return beKA::beStatus_Invalid;
}

// Checks if the required output files are generated by the amdspv.
// Only verifies the files requested in the "options.m_pipelineShaders" name list.
static bool  VerifyAmdspvOutput(const VulkanOptions& options)
{
    bool  ret = true;
    if (options.m_isAmdIsaDisassemblyRequired)
    {
        ret &= (options.m_pipelineShaders.m_computeShader.isEmpty()        || beUtils::isFilePresent(options.m_isaDisassemblyOutputFiles.m_computeShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_fragmentShader.isEmpty()       || beUtils::isFilePresent(options.m_isaDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_geometryShader.isEmpty()       || beUtils::isFilePresent(options.m_isaDisassemblyOutputFiles.m_geometryShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_tessControlShader.isEmpty()    || beUtils::isFilePresent(options.m_isaDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_tessEvaluationShader.isEmpty() || beUtils::isFilePresent(options.m_isaDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_vertexShader.isEmpty()         || beUtils::isFilePresent(options.m_isaDisassemblyOutputFiles.m_vertexShader.asASCIICharArray()));
    }
    if (ret && options.m_isAmdIsaBinariesRequired)
    {
        ret &= (options.m_pipelineShaders.m_computeShader.isEmpty()        || beUtils::isFilePresent(options.m_isaBinaryFiles.m_computeShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_fragmentShader.isEmpty()       || beUtils::isFilePresent(options.m_isaBinaryFiles.m_fragmentShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_geometryShader.isEmpty()       || beUtils::isFilePresent(options.m_isaBinaryFiles.m_geometryShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_tessControlShader.isEmpty()    || beUtils::isFilePresent(options.m_isaBinaryFiles.m_tessControlShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_tessEvaluationShader.isEmpty() || beUtils::isFilePresent(options.m_isaBinaryFiles.m_tessEvaluationShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_vertexShader.isEmpty()         || beUtils::isFilePresent(options.m_isaBinaryFiles.m_vertexShader.asASCIICharArray()));
    }
    if (ret && options.m_isSpirvBinariesRequired)
    {
        ret &= (options.m_pipelineShaders.m_computeShader.isEmpty()        || beUtils::isFilePresent(STR_COMP_SPV_OUTPUT_FILE_NAME));
        ret &= (options.m_pipelineShaders.m_fragmentShader.isEmpty()       || beUtils::isFilePresent(STR_FRAG_SPV_OUTPUT_FILE_NAME));
        ret &= (options.m_pipelineShaders.m_geometryShader.isEmpty()       || beUtils::isFilePresent(STR_GEOM_SPV_OUTPUT_FILE_NAME));
        ret &= (options.m_pipelineShaders.m_tessControlShader.isEmpty()    || beUtils::isFilePresent(STR_TESC_SPV_OUTPUT_FILE_NAME));
        ret &= (options.m_pipelineShaders.m_tessEvaluationShader.isEmpty() || beUtils::isFilePresent(STR_TESE_SPV_OUTPUT_FILE_NAME));
        ret &= (options.m_pipelineShaders.m_vertexShader.isEmpty()         || beUtils::isFilePresent(STR_VERT_SPV_OUTPUT_FILE_NAME));
    }
    if (ret && options.m_isAmdPalIlBinariesRequired)
    {
        ret &= (options.m_pipelineShaders.m_computeShader.isEmpty()        || beUtils::isFilePresent(STR_COMP_PALIL_OUTPUT_FILE_NAME));
        ret &= (options.m_pipelineShaders.m_fragmentShader.isEmpty()       || beUtils::isFilePresent(STR_FRAG_PALIL_OUTPUT_FILE_NAME));
        ret &= (options.m_pipelineShaders.m_geometryShader.isEmpty()       || beUtils::isFilePresent(STR_GEOM_PALIL_OUTPUT_FILE_NAME));
        ret &= (options.m_pipelineShaders.m_tessControlShader.isEmpty()    || beUtils::isFilePresent(STR_TESC_PALIL_OUTPUT_FILE_NAME));
        ret &= (options.m_pipelineShaders.m_tessEvaluationShader.isEmpty() || beUtils::isFilePresent(STR_TESE_PALIL_OUTPUT_FILE_NAME));
        ret &= (options.m_pipelineShaders.m_vertexShader.isEmpty()         || beUtils::isFilePresent(STR_VERT_PALIL_OUTPUT_FILE_NAME));
    }
    if (ret && options.m_isAmdPalIlDisassemblyRequired)
    {
        ret &= (options.m_pipelineShaders.m_computeShader.isEmpty()        || beUtils::isFilePresent(options.m_pailIlDisassemblyOutputFiles.m_computeShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_fragmentShader.isEmpty()       || beUtils::isFilePresent(options.m_pailIlDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_geometryShader.isEmpty()       || beUtils::isFilePresent(options.m_pailIlDisassemblyOutputFiles.m_geometryShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_tessControlShader.isEmpty()    || beUtils::isFilePresent(options.m_pailIlDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_tessEvaluationShader.isEmpty() || beUtils::isFilePresent(options.m_pailIlDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_vertexShader.isEmpty()         || beUtils::isFilePresent(options.m_pailIlDisassemblyOutputFiles.m_vertexShader.asASCIICharArray()));
    }
    if (ret && options.m_isScStatsRequired)
    {
        ret &= (options.m_pipelineShaders.m_computeShader.isEmpty()        || beUtils::isFilePresent(options.m_scStatisticsOutputFiles.m_computeShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_fragmentShader.isEmpty()       || beUtils::isFilePresent(options.m_scStatisticsOutputFiles.m_fragmentShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_geometryShader.isEmpty()       || beUtils::isFilePresent(options.m_scStatisticsOutputFiles.m_geometryShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_tessControlShader.isEmpty()    || beUtils::isFilePresent(options.m_scStatisticsOutputFiles.m_tessControlShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_tessEvaluationShader.isEmpty() || beUtils::isFilePresent(options.m_scStatisticsOutputFiles.m_tessEvaluationShader.asASCIICharArray()));
        ret &= (options.m_pipelineShaders.m_vertexShader.isEmpty()         || beUtils::isFilePresent(options.m_scStatisticsOutputFiles.m_vertexShader.asASCIICharArray()));
    }

    return ret;
}

beKA::beStatus beProgramBuilderVulkan::Compile(const VulkanOptions& vulkanOptions, bool& cancelSignal, bool printCmd, gtString& buildLog)
{
    GT_UNREFERENCED_PARAMETER(cancelSignal);
    beKA::beStatus ret = beKA::beStatus_General_FAILED;
    buildLog.makeEmpty();

    // Get amdspv's path.
    std::string ambdbilPath;
    GetAmdspvPath(ambdbilPath);

    AMDTDeviceInfoUtils* pDeviceInfo = AMDTDeviceInfoUtils::Instance();

    if (pDeviceInfo != nullptr)
    {
        // Numerical representation of the HW generation.
        std::string deviceGfxIp;

        // Convert the HW generation to the amdspv string.
        bool isDeviceHwGenExtracted = GetGfxIpForVulkan(pDeviceInfo, vulkanOptions, deviceGfxIp);

        if (isDeviceHwGenExtracted && !deviceGfxIp.empty())
        {
            // Build the command for invoking amdspv.
            std::stringstream cmd;
            cmd << ambdbilPath;

            if (vulkanOptions.m_optLevel != -1)
            {
                cmd << " -O" << std::to_string(vulkanOptions.m_optLevel) << " ";
            }

            cmd << " -Dall -l -gfxip " << deviceGfxIp << " -set ";

            if ((ret = AddInputFileNames(vulkanOptions, cmd)) == beKA::beStatus_SUCCESS)
            {
                AddOutputFileNames(vulkanOptions, cmd);

                // Redirect build log to a temporary file.
                const gtString AMPSPV_TMP_OUTPUT_FILE = L"amdspvTempFile.txt";
                osFilePath tmpFilePath(osFilePath::OS_TEMP_DIRECTORY);
                tmpFilePath.setFileName(AMPSPV_TMP_OUTPUT_FILE);

                // Delete the log file if it already exists.
                if (tmpFilePath.exists())
                {
                    osFile tmpLogFile(tmpFilePath);
                    tmpLogFile.deleteFile();
                }

                cmd << "out.glslLog=\"" << tmpFilePath.asString().asASCIICharArray() << "\" ";

                // No default output (only generate the output files that we explicitly specified).
                cmd << "defaultOutput=0";

                // Launch amdspv.
                gtString amdspvOutput;
                beUtils::PrintCmdLine(cmd.str(), printCmd);
                bool isLaunchSuccess = osExecAndGrabOutput(cmd.str().c_str(), cancelSignal, amdspvOutput);

                if (isLaunchSuccess)
                {
                    // This is how amdspv signals success.
                    const gtString AMDSPV_SUCCESS_TOKEN = L"SUCCESS!";

                    // Check if the output files were generated and amdspv returned "success".
                    if (amdspvOutput.find(AMDSPV_SUCCESS_TOKEN) == std::string::npos)
                    {
                        ret = beKA::beStatus_VulkanAmdspvCompilationFailure;

                        // Read the build log.
                        if (tmpFilePath.exists())
                        {
                            // Read the build log.
                            gtString compilerOutput;
                            std::ifstream file(tmpFilePath.asString().asASCIICharArray());
                            std::string tmpCmdOutput((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                            buildLog << tmpCmdOutput.c_str();

                            // Delete the temporary file.
                            osFile fileToDelete(tmpFilePath);
                            fileToDelete.deleteFile();
                        }

                        // Let's end the build log with the error that was provided by the backend.
                        if (!amdspvOutput.isEmpty())
                        {
                            buildLog << "Error: " << amdspvOutput << L"\n";
                        }
                    }
                    else if (!VerifyAmdspvOutput(vulkanOptions))
                    {
                        ret = beKA::beStatus_FailedOutputVerification;
                    }
                    else
                    {
                        ret = beKA::beStatus_SUCCESS;

                        // Delete the ISA binaries if they are not required.
                        if (!vulkanOptions.m_isAmdIsaBinariesRequired)
                        {
                            beUtils::DeleteOutputFiles(vulkanOptions.m_isaBinaryFiles);
                        }
                    }
                }
                else
                {
                    ret = beKA::beStatus_VulkanAmdspvLaunchFailure;
                }
            }
        }
        else
        {
            ret = beKA::beStatus_GLUnknownHardwareFamily;
        }
    }

    return ret;
}


bool beProgramBuilderVulkan::GetVulkanVersion(gtString& vkVersion)
{
    vkVersion = BE_STR_VULKAN_VERSION;
    return true;
}

bool beProgramBuilderVulkan::GetSupportedDevices(std::set<std::string>& deviceList)
{
    std::vector<GDT_GfxCardInfo> tmpCardList;
    bool ret = beUtils::GetAllGraphicsCards(tmpCardList, deviceList);
    return ret;
}
