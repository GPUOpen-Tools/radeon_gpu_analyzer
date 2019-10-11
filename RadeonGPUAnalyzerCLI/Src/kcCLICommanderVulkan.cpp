//=================================================================
// Copyright 2018 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#include <cassert>
#include <fstream>
#include <iterator>
#include <array>
#include <map>
#include <fstream>
#include <algorithm>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4309)
#endif
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <Core/Vulkan/tools/include/spirv_cross/spirv_cross.hpp>
#ifdef _WIN32
#pragma warning(pop)
#endif

// Shared.
#include <Utils/Include/rgaCliDefs.h>
#include <Utils/Include/rgLog.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderVulkan.h>
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>
#include <RadeonGPUAnalyzerBackend/Emulator/Parser/ISAParser.h>
#include <RadeonGPUAnalyzerBackend/Include/beStringConstants.h>

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderVulkan.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderVkOffline.h>
#include <RadeonGPUAnalyzerCLI/Src/kcXmlWriter.h>

using namespace beKA;

// Constants.

// Magic number for SPIR-V binary files (SPIR-V Spec, par. 3.1)
static const uint32_t  SPV_BINARY_MAGIC_NUMBER = spv::MagicNumber;

// Extensions of different input/output file types.
static const std::string  STR_VULKAN_SPIRV_FILE_EXT           = "spv";
static const std::string  STR_VULKAN_HLSL_FILE_EXT            = "hlsl";
static const std::string  STR_VULKAN_BIN_FILE_EXT             = "bin";
static const std::string  STR_VULKAN_ISA_FILE_EXT             = "isa";
static const std::string  STR_VULKAN_STATS_FILE_SUFFIX        = "stats";
static const std::string  STR_VULKAN_STATS_FILE_EXT           = "csv";
static const std::string  STR_VULKAN_VALIDATION_INFO_FILE_EXT = "txt";

// Vulkan statistics tags.
static const std::string  STR_VULKAN_STATS_TITLE              = "Statistics:";
static const std::string  STR_VULKAN_STATS_TAG_NUM_USED_VGPRS = "resourceUsage.numUsedVgprs";
static const std::string  STR_VULKAN_STATS_TAG_NUM_USED_SGPRS = "resourceUsage.numUsedSgprs";
static const std::string  STR_VULKAN_STATS_TAG_NUM_AVL_VGPRS  = "numAvailableVgprs";
static const std::string  STR_VULKAN_STATS_TAG_NUM_AVL_SGPRS  = "numAvailableSgprs";
static const std::string  STR_VULKAN_STATS_TAG_LDS_SIZE       = "resourceUsage.ldsSizePerLocalWorkGroup";
static const std::string  STR_VULKAN_STATS_TAG_LDS_USAGE      = "resourceUsage.ldsUsageSizeInBytes";
static const std::string  STR_VULKAN_STATS_TAG_SCRATCH_MEM    = "resourceUsage.scratchMemUsageInBytes";

// Default name for temporary files.
static const std::string  STR_VULKAN_TEMP_VALIDATION_INFO_FILE_NAME = "rga-vk-validation-info";

// Temp file suffix for all devices.
static const std::string  STR_VULKAN_FILE_SUFFIX_ALL_DEVICES = "all-devices";

// Suffixes for stage-specific output files.
static const std::array<std::string, bePipelineStage::Count>
STR_VULKAN_STAGE_FILE_SUFFIXES_DEFAULT =
{
    "vert",
    "tesc",
    "tese",
    "geom",
    "frag",
    "comp"
};

// Suffixes for stage-specific output files - cont.
// We need to keep these separate from the default ones
// since the default container is being accessed using the
// [] operator and are the items are assumed to be in order.
static const std::vector<std::string>
STR_VULKAN_STAGE_FILE_SUFFIXES_1 =
{
    "vs",
    "fs",
    "glsl"
};

// The name of the VK_LOADER_DEBUG environment variable.
static const std::string  STR_VULKAN_VK_LOADER_DEBUG_ENV_VAR_NAME = "VK_LOADER_DEBUG";

// Map:  SPIR-V execution model --> RGA pipeline stage.
static const std::map<spv::ExecutionModel, bePipelineStage>
PIPELINE_STAGE_FOR_SPV_EXEC_MODEL =
{
    {spv::ExecutionModel::ExecutionModelFragment,               bePipelineStage::Fragment},
    {spv::ExecutionModel::ExecutionModelGeometry,               bePipelineStage::Geometry},
    {spv::ExecutionModel::ExecutionModelGLCompute,              bePipelineStage::Compute},
    {spv::ExecutionModel::ExecutionModelTessellationControl,    bePipelineStage::TessellationControl},
    {spv::ExecutionModel::ExecutionModelTessellationEvaluation, bePipelineStage::TessellationEvaluation},
    {spv::ExecutionModel::ExecutionModelVertex,                 bePipelineStage::Vertex}
};

// File extensions for GLSL source files - default.
static const std::array<std::string, bePipelineStage::Count>
STR_GLSL_FILE_EXTENSIONS = STR_VULKAN_STAGE_FILE_SUFFIXES_DEFAULT;

// File extensions for GLSL source files - additional.
static const std::vector<std::string>
STR_GLSL_FILE_EXTENSIONS_ADDITIONAL = STR_VULKAN_STAGE_FILE_SUFFIXES_1;


// Pipeline stage names for parsed SPIR-V info.
static const std::array<std::string, bePipelineStage::Count>
STR_PIPELINE_STAGE_NAMES = STR_VULKAN_STAGE_FILE_SUFFIXES_DEFAULT;

// Pipeline stage full names used in text messages.
static const std::array<std::string, bePipelineStage::Count>
STR_PIPELINE_STAGE_FULL_NAMES =
{
    KA_CLI_STR_VERTEX_SHADER,
    KA_CLI_STR_TESS_CTRL_SHADER,
    KA_CLI_STR_TESS_EVAL_SHADER,
    KA_CLI_STR_GEOMETRY_SHADER,
    KA_CLI_STR_FRAGMENT_SHADER,
    KA_CLI_STR_COMPUTE_SHADER
};

// File extensions for HLSL, SPIRV binary and SPIRV text files.
static const std::string  STR_HLSL_FILE_EXTENSION = "hlsl";
static const std::string  STR_SPIRV_FILE_EXTENSION = "spv";
static const std::string  STR_SPIRV_TXT_FILE_EXTENSION = "spvas";

// Output metadata entry types for OpenGL pipeline stages.
static const std::array<rgEntryType, bePipelineStage::Count>
VULKAN_OGL_STAGE_ENTRY_TYPES =
{
    rgEntryType::GL_Vertex,
    rgEntryType::GL_TessControl,
    rgEntryType::GL_TessEval,
    rgEntryType::GL_Geometry,
    rgEntryType::GL_Fragment,
    rgEntryType::GL_Compute
};

// HLSL text parser.
// Used to look through specified HLSL program text and extract required data.
// Note: the HLSL text must be preprocessed for accurate results.
class kcHlslParser
{
public:
    // C-tor
    kcHlslParser(const std::string& text) : m_text(text), m_textSize(text.size()) {}

    // Extract the function names.
    bool GetFuncNames(std::vector<std::string>& names)
    {
        int scopeDepth = 0;
        char sym;
        size_t pos = 0;

        while ((pos = Fetch(sym)) != TEXT_END)
        {
            switch (sym)
            {
            case '{': scopeDepth++; break;
            case '}': scopeDepth--; break;
            case '(':
                if (scopeDepth == 0)
                {
                    // Store current position and look for matching ')'.
                    size_t parOpen = pos;
                    bool stop = false;
                    int parDepth = 1;

                    while (!stop && (pos = Fetch(sym)) != TEXT_END)
                    {
                        parDepth += (sym == '(' ? 1 : (sym == ')' ? -1 : 0));
                        stop = (sym == ')' && parDepth == 0 ? true : false);
                    }

                    if (stop && sym == ')')
                    {
                        // The function arglist may be followed by a "semantic" specifier, for example:
                        // float4 psMainD3D10( float4 screenSpace : SV_Position ) : COLOR {...}
                        if (Fetch(sym) != TEXT_END)
                        {
                            if (sym == '{' ||
                                (sym == ':' && !FetchToken().empty() && Fetch(sym) != TEXT_END && sym == '{'))
                            {
                                MoveCaret(parOpen);
                                std::string name = RFetchToken();

                                // If the token preceeding the found name is ':', it's a type specifier, not a function name.
                                if (RFetchToken() != ":")
                                {
                                    // We found a function definition.
                                    names.push_back(name);
                                }

                                // Advance the caret position to ")" symbol to continue search.
                                MoveCaret(pos);
                            }
                        }
                    }
                }
                break;
            }
        }

        return (scopeDepth == 0);
    }

private:
    // Checks if symbol is space, tab or newline.
    inline bool IsSpace(char sym)
    {
        return (sym == ' ' || sym == '\t' || sym == '\n');
    }

    // Fetch one symbol and increment the caret position. The leading spaces/tabs/newlines are skipped.
    // Returns old caret position or TEXT_END if end of text reached.
    inline size_t Fetch(char& sym)
    {
        while (m_caret < m_textSize && IsSpace(m_text[m_caret])) { m_caret++; }
        sym = (m_caret < m_textSize ? m_text[m_caret] : 0);
        return (m_caret < m_textSize ? m_caret++ : TEXT_END);
    }

    // Skips spaces/tabs/newlines. Shifts the caret position accordingly.
    // Returns updated caret position or TEXT_END if end of text is reached.
    size_t SkipSpaces()
    {
        while (m_caret < m_textSize && IsSpace(m_text[m_caret])) { m_caret++; }
        return (m_caret < m_textSize ? m_caret : TEXT_END);
    }

    // Skips spaces/tabs/newlines in reverse direction. Shifts the caret position accordingly.
    // Returns updated caret position or TEXT_END if beginning of the text is reached.
    // The returned caret position points to the last space before a token (in reverse order).
    size_t RSkipSpaces()
    {
        while (m_caret > 0 && IsSpace(m_text[--m_caret])) {}
        return (m_caret + 1 > 0 ? ++m_caret : TEXT_END);
    }

    // Sets caret to the required position.
    void MoveCaret(size_t pos)
    {
        assert(pos < m_textSize);
        m_caret = (pos < m_textSize ? pos : m_caret);
    }

    // Fetches a token that consisting of letters and numbers, skipping the leading spaces.
    // If the symbol at current caret position is not a letter or number, returns this symbol.
    // If the end of text is reached, returns empty string.
    // Advances the caret position accordingly.
    std::string FetchToken()
    {
        std::string ret;
        if (SkipSpaces() != TEXT_END)
        {
            while (m_caret < m_textSize &&
                ((m_text[m_caret] >= 'A' && m_text[m_caret] <= 'z') ||
                (m_text[m_caret] >= '0' && m_text[m_caret] <= '9') || m_text[m_caret] == '_'))
            {
                ret.push_back(m_text[m_caret++]);
            }

            if (ret.empty() && m_caret < m_textSize)
            {
                ret.push_back(m_text[m_caret++]);
            }
        }

        return ret;
    }

    // Fetches a token that consisting of letters and numbers, skipping the leading spaces going in reverse direction.
    // Search starts with symbol preceding the current caret position.
    // If the 1st checked symbol is not a letter or number, returns this symbol.
    // If the end of text is reached, returns empty string.
    // Adjusts the caret position accordingly.
    std::string RFetchToken()
    {
        std::string ret;
        if (RSkipSpaces() != TEXT_END)
        {
            m_caret--;
            while (m_caret + 1 > 0 &&
                ((m_text[m_caret] >= 'A' && m_text[m_caret] <= 'z') ||
                (m_text[m_caret] >= '0' && m_text[m_caret] <= '9') || m_text[m_caret] == '_'))
            {
                ret.push_back(m_text[m_caret--]);
            }

            if (ret.empty() && m_caret > 0)
            {
                ret.push_back(m_text[m_caret]);
            }
            else
            {
                m_caret++;
            }
        }

        std::reverse(ret.begin(), ret.end());

        return ret;
    }

    std::string  m_text;
    size_t       m_textSize = 0;
    size_t       m_caret = 0;
    const size_t TEXT_END = std::numeric_limits<size_t>::max();
};

// Callback for printing to stdout.
static void loggingCallback(const string& s)
{
    rgLog::stdOut << s.c_str() << std::flush;
}


// Construct per-stage output file names for binary, ISA disassembly and statistics based on base file
// names specified by a user and target GPU name ("device"). "spvFileNames" contains the names of input
// spv files. The output ISA file names will be generated for corresponding non-empty spv file names.
static bool ConstructVkOutputFileNames(const std::string&       baseBinFileName,
    const std::string&       baseIsaFileName,
    const std::string&       baseStatsFileName,
    const std::string&       device,
    const beVkPipelineFiles& spvFileNames,
    std::string&             binFileName,
    beVkPipelineFiles&       isaFileNames,
    beVkPipelineFiles&       statsFileNames)
{
    bool status = true;

    if (!baseBinFileName.empty())
    {
        status = kcUtils::ConstructOutFileName(baseBinFileName, "", device, STR_VULKAN_BIN_FILE_EXT, binFileName);
    }

    for (int stage = 0; stage < bePipelineStage::Count && status; stage++)
    {
        if (!spvFileNames[stage].empty() && status)
        {
            status = status && kcUtils::ConstructOutFileName(baseIsaFileName, STR_VULKAN_STAGE_FILE_SUFFIXES_DEFAULT[stage], device,
                STR_VULKAN_ISA_FILE_EXT, isaFileNames[stage]);

            status = status && kcUtils::ConstructOutFileName(baseStatsFileName, STR_VULKAN_STAGE_FILE_SUFFIXES_DEFAULT[stage], device,
                STR_VULKAN_STATS_FILE_EXT, statsFileNames[stage]);
        }
    }

    assert(status);

    return status;
}

// Construct output SPIR-V file name based on the provided input file name.
static std::string ConstructOutputSpirvFileName(const std::string& inputFileName)
{
    gtString gInputFile;
    gInputFile << inputFileName.c_str();
    osFilePath  spvFilePath(gInputFile);
    gtString  gSpvFileExt;
    gSpvFileExt << STR_VULKAN_SPIRV_FILE_EXT.c_str();
    spvFilePath.setFileExtension(gSpvFileExt);
    return spvFilePath.asString().asASCIICharArray();
}

static void  LogPreStep(const std::string& msg, const std::string& device = "")
{
    rgLog::stdOut << msg << device << "... " << std::flush;
}

static void  LogResult(bool result)
{
    rgLog::stdOut << (result ? KA_CLI_STR_STATUS_SUCCESS : KA_CLI_STR_STATUS_FAILURE) << std::endl;
}

static void  LogErrorStatus(beStatus status, const std::string& errMsg)
{
    switch (status)
    {
    case beStatus_SUCCESS:
        break;
    case beStatus_Vulkan_GlslangLaunchFailed:
        rgLog::stdOut << STR_ERR_VULKAN_GLSLANG_LAUNCH_FAILED << std::endl;
        break;
    case beStatus_Vulkan_FrontendCompileFailed:
        rgLog::stdOut << STR_ERR_VULKAN_FRONTENDEND_FAILED << std::endl;
        if (!errMsg.empty())
        {
            rgLog::stdOut << rgLog::noflush << STR_ERR_VULKAN_GLSLANG_ERROR << std::endl;
            rgLog::stdOut << errMsg << std::endl << rgLog::flush;
        }
        break;
    case beStatus_Vulkan_BackendLaunchFailed:
        rgLog::stdOut << STR_ERR_VULKAN_BACKEND_LAUNCH_FAILED << std::endl;
        break;
    case beStatus_Vulkan_BackendCompileFailed:
        rgLog::stdOut << STR_ERR_VULKAN_BACKEND_FAILED << std::endl;
        if (!errMsg.empty())
        {
            rgLog::stdOut << rgLog::noflush << STR_ERR_VULKAN_BACKEND_ERROR << std::endl;
            rgLog::stdOut << errMsg << std::endl << rgLog::flush;
        }
        break;
    case beStatus_ConstructIsaFileNameFailed:
        rgLog::stdOut << STR_ERR_FAILED_ADJUST_FILE_NAMES << std::endl;
        break;
    case beStatus_Vulkan_ParseStatsFailed:
        rgLog::stdOut << STR_ERR_FAILED_CONVERT_VULKAN_STATS << std::endl;
        break;
    default:
        std::cout << std::endl << (errMsg.empty() ? STR_ERR_UNKNOWN_COMPILATION_STATUS : errMsg) << std::endl;
        break;
    }
}

// Parse the content of Vulkan stats and store values to "data" structure.
static bool ParseVulkanStats(const std::string isaText, const std::string& statsText, beKA::AnalysisData& data)
{
    bool result = false;
    std::string line, tag, dash, equals;
    std::stringstream sText(statsText), sLine;
    CALuint64 value;

    // Read the statistics text line by line and parse each line.
    // Skip the 1st line which is the title.
    if ((result = std::getline(sText, line) && line == STR_VULKAN_STATS_TITLE) == true)
    {
        while (result && std::getline(sText, line))
        {
            sLine.clear();
            sLine << line;
            sLine >> dash >> tag >> equals >> value;
            if ((result = (dash == "-" && equals == "=")) == true)
            {
                if (tag == STR_VULKAN_STATS_TAG_NUM_USED_VGPRS)
                {
                    data.numVGPRsUsed = value;
                }
                else if (tag == STR_VULKAN_STATS_TAG_NUM_AVL_VGPRS)
                {
                    data.numVGPRsAvailable = value;
                }
                else if (tag == STR_VULKAN_STATS_TAG_NUM_USED_SGPRS)
                {
                    data.numSGPRsUsed = value;
                }
                else if (tag == STR_VULKAN_STATS_TAG_NUM_AVL_SGPRS)
                {
                    data.numSGPRsAvailable = value;
                }
                else if (tag == STR_VULKAN_STATS_TAG_LDS_SIZE)
                {
                    data.LDSSizeAvailable = value;
                }
                else if (tag == STR_VULKAN_STATS_TAG_LDS_USAGE)
                {
                    data.LDSSizeUsed = value;
                }
                else if (tag == STR_VULKAN_STATS_TAG_SCRATCH_MEM)
                {
                    data.scratchMemoryUsed = value;
                }
            }
        }
    }

    // Add the ISA size.
    assert(result);
    if (result)
    {
        ParserISA isaParser;

        if ((result = isaParser.ParseForSize(isaText)) == true)
        {
            data.ISASize = isaParser.GetCodeLen();
        }
    }

    assert(result);

    return result;
}

// Convert statistics file from Vulkan mode into normal RGA stats format.
static beStatus ConvertStats(const beVkPipelineFiles& isaFiles, const beVkPipelineFiles& statsFiles,
    const Config& config, const std::string& device)
{
    beStatus status = beStatus_SUCCESS;

    for (int stage = 0; stage < bePipelineStage::Count && status == beStatus_SUCCESS; stage++)
    {
        if (!statsFiles[stage].empty())
        {
            bool result = false;
            std::string statsText, isaText;
            auto logFunc = [](const std::string& s) { rgLog::stdOut << s; };

            if (((result = kcUtils::ReadTextFile(statsFiles[stage], statsText, logFunc) == true) &&
                ((result = kcUtils::ReadTextFile(isaFiles[stage], isaText, logFunc)) == true)))
            {
                beKA::AnalysisData statsData;
                if ((result = ParseVulkanStats(isaText, statsText, statsData)) == true)
                {
                    gtString gFileName;
                    gFileName << statsFiles[stage].c_str();
                    kcUtils::CreateStatisticsFile(gFileName, config, device, statsData, nullptr);
                    result = (kcUtils::FileNotEmpty(statsFiles[stage]));
                }
            }
            status = (result ? beStatus_SUCCESS : beStatus_Vulkan_ParseStatsFailed);
        }
    }

    return status;
}

// Checks if specified file path is a SPIR-V binary file.
static bool IsSpvBinFile(const std::string& filePath)
{
    bool isSpv = false;

    if (kcUtils::FileNotEmpty(filePath))
    {
        std::ifstream file(filePath);
        if (file.good())
        {
            // Read the first 32-bit word of the file and check if it matches the SPIR-V binary magic number.
            uint32_t word;
            if (file.read(reinterpret_cast<char*>(&word), sizeof(word)) && file.good())
            {
                isSpv = (word == SPV_BINARY_MAGIC_NUMBER);
            }
        }
    }

    return isSpv;
}


// Extract the list of entry points and corresponding execution models from the SPIR-V binary file specified by "spvFilePath".
// Extracted entry points are returned in the "entries" vector.
// Returns "true" if succeeded or "false" otherwise.
static bool ExtractSpvEntries(const std::string& spvFilePath, std::vector<spirv_cross::EntryPoint>& entries)
{
    bool result = false;
    entries.clear();

    FILE *spvFile = std::fopen(spvFilePath.c_str(), "rb");
    if (spvFile != nullptr)
    {
        std::fseek(spvFile, 0L, SEEK_END);
        std::vector<uint32_t> spv(std::ftell(spvFile) / sizeof(uint32_t));
        std::rewind(spvFile);

        result = (std::fread(spv.data(), sizeof(uint32_t), spv.size(), spvFile) == spv.size());

        if (result)
        {
            spirv_cross::Compiler cmplr(spv);
            entries = cmplr.get_entry_points_and_stages();
            result = true;
        }
    }

    return result;
}

// Get the type of Vulkan input type corresponding to the provided file extension:
//  "hlsl" --> HLSL.
//  "spv"  --> SPIR-V binary.
//  "spvas" --> SPIR-V text.
//  "vert", "frag", etc. --> GLSL.
//  other --> SPIR-V binary.
static rgVulkanInputType GetInputTypeForExt(std::string fileExt)
{
    rgVulkanInputType inputType = rgVulkanInputType::Unknown;
    if (!fileExt.empty())
    {
        std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), [](const char& c) {return std::tolower(c); });

        if (std::find(STR_GLSL_FILE_EXTENSIONS.cbegin(), STR_GLSL_FILE_EXTENSIONS.cend(), fileExt) != STR_GLSL_FILE_EXTENSIONS.cend() ||
            std::find(STR_GLSL_FILE_EXTENSIONS_ADDITIONAL.cbegin(), STR_GLSL_FILE_EXTENSIONS_ADDITIONAL.cend(), fileExt) != STR_GLSL_FILE_EXTENSIONS_ADDITIONAL.cend())
        {
            inputType = rgVulkanInputType::Glsl;
        }
        else if (fileExt == STR_HLSL_FILE_EXTENSION)
        {
            inputType = rgVulkanInputType::Hlsl;
        }
        else if (fileExt == STR_SPIRV_FILE_EXTENSION)
        {
            inputType = rgVulkanInputType::Spirv;
        }
        else if (fileExt == STR_SPIRV_TXT_FILE_EXTENSION)
        {
            inputType = rgVulkanInputType::SpirvTxt;
        }
        else
        {
            inputType = rgVulkanInputType::Spirv;
        }
    }

    return inputType;
}

// Detect the type of the given Vulkan input file by its extension.
static rgVulkanInputType DetectInputTypeByFileExt(const std::string& filePath)
{
    rgVulkanInputType inputType = rgVulkanInputType::Unknown;

    // Get the file extension.
    size_t offset = filePath.rfind('.');
    const std::string& ext = (offset != std::string::npos && ++offset < filePath.size()) ? filePath.substr(offset) : "";

    // Try to detect the type of shader file by its extension.
    inputType = GetInputTypeForExt(ext);
    if (inputType == rgVulkanInputType::Unknown)
    {
        rgLog::stdOut << STR_ERR_VULKAN_CANNOT_DETECT_INPUT_FILE_BY_EXT_1 << std::endl;
    }

    return inputType;
}

// Detect the type of vulkan input files.
// Returns an array of input shader types corresponding to the pipeline stages.
static std::array<rgVulkanInputType, bePipelineStage::Count>
DetectInputFiles(const Config& config)
{
    std::array<rgVulkanInputType, bePipelineStage::Count> ret;
    ret.fill(rgVulkanInputType::Unknown);

    // Identify the type of input files.
    // 1. If the input types for all stages is specified explicitly ("--hlsl", "--spv", "--spvtxt" options), assume that type.
    if (config.m_isHlslInput)
    {
        ret.fill(rgVulkanInputType::Hlsl);
    }
    else if (config.m_isSpvInput)
    {
        ret.fill(rgVulkanInputType::Spirv);
    }
    else if (config.m_isSpvTxtInput)
    {
        ret.fill(rgVulkanInputType::Hlsl);
    }
    else
    {
        // 2. If the input type for all stages is not explicitly specified.
        std::array<rgVulkanInputType, bePipelineStage::Count> userFileTypes =
        { config.m_vertShaderFileType, config.m_tescShaderFileType, config.m_teseShaderFileType,
          config.m_geomShaderFileType, config.m_fragShaderFileType, config.m_compShaderFileType };

        beVkPipelineFiles files = { config.m_VertexShader, config.m_TessControlShader, config.m_TessEvaluationShader,
                                    config.m_GeometryShader, config.m_FragmentShader, config.m_ComputeShader };

        // Check if per-stage file type is provided (for example, the "--vert-glsl" option is used).
        // If not, try to detect the shader file type by its extension:
        //  "hlsl" --> HLSL.
        //  "spvas" --> SPIR-V text.
        //  "vert", "frag", etc. --> GLSL.
        //  other  --> SPIR-V binary.
        for (int stage = bePipelineStage::Vertex; stage < bePipelineStage::Count; ++stage)
        {
            if (!files[stage].empty())
            {
                ret[stage] = (userFileTypes[stage] != rgVulkanInputType::Unknown ?
                    userFileTypes[stage] : DetectInputTypeByFileExt(files[stage]));
            }
        }
    }

    return ret;
}

void kcCLICommanderVulkan::RunCompileCommands(const Config& config, LoggingCallBackFunc_t)
{
    bool status = false;
    bool shouldAbort = false;
    std::string  spvFileName;

    bool isInputFileRequired = (!config.m_ISAFile.empty() ||
        !config.m_BinaryOutputFile.empty() ||
        !config.m_AnalysisFile.empty() ||
        !config.m_LiveRegisterAnalysisFile.empty() ||
        !config.m_blockCFGFile.empty() ||
        !config.m_instCFGFile.empty());

    if (isInputFileRequired &&
        config.m_VertexShader.empty() &&
        config.m_TessControlShader.empty() &&
        config.m_TessEvaluationShader.empty() &&
        config.m_GeometryShader.empty() &&
        config.m_FragmentShader.empty() &&
        config.m_ComputeShader.empty())
    {
        std::cout << STR_ERR_NO_INPUT_FILE << std::endl;
        shouldAbort = true;
    }

    if (!config.m_psoDx12.empty())
    {
        std::cout << STR_ERR_VULKAN_GPSO_OPTION_NOT_SUPPORTED << std::endl;
        shouldAbort = true;
    }

    if (!shouldAbort)
    {
        if (!config.m_icdFile.empty())
        {
            // If custom ICD location was used, notify the user.
            std::cout << STR_INFO_USING_CUSTOM_ICD_FILE << config.m_icdFile << std::endl << std::endl;
        }

        if (!config.m_loaderDebug.empty())
        {
            bool isEnvVarSet = kcUtils::SetEnvrironmentVariable(STR_VULKAN_VK_LOADER_DEBUG_ENV_VAR_NAME, config.m_loaderDebug);
            assert(isEnvVarSet);
            if (!isEnvVarSet)
            {
                // Notify the user.
                std::cout << STR_WRN_VULKAN_FAILED_SET_ENV_VAR_A << STR_VULKAN_VK_LOADER_DEBUG_ENV_VAR_NAME << " " <<
                    STR_WRN_VULKAN_FAILED_SET_ENV_VAR_B << std::endl;
            }
        }

        // Detect the action requested by user.
        if (!config.m_spvBin.empty())
        {
            // Assembling SPIR-V text is required.
            status = AssembleSpv(config);
        }
        else if (!config.m_spvTxt.empty())
        {
            // Disassembling SPIR-V binary is requested.
            status = DisassembleSpv(config);
        }
        else if (!config.m_parsedSpv.empty())
        {
            // Parsing SPIR-V binary is requested.
            status = ParseSpv(config);
        }
        else
        {
            if ((status = InitRequestedAsicListVulkan(config)) == true)
            {
                // Detect the type of input files.
                auto inputFileTypes = DetectInputFiles(config);

                beVkPipelineFiles inputFiles = { config.m_VertexShader, config.m_TessControlShader, config.m_TessEvaluationShader,
                                                 config.m_GeometryShader, config.m_FragmentShader, config.m_ComputeShader };

                beVkPipelineFiles glslFiles, hlslFiles, spvTxtFiles, spvFiles;
                bool foundGslsHlslFiles = false, foundSpvTxtFiles = false;

                // Collect shader files that have to be pre-compiled to SPIR-V binary format.
                for (int stage = bePipelineStage::Vertex; stage < bePipelineStage::Count; ++stage)
                {
                    if (inputFileTypes[stage] == rgVulkanInputType::Glsl)
                    {
                        glslFiles[stage] = inputFiles[stage];
                        foundGslsHlslFiles = true;
                    }
                    else if (inputFileTypes[stage] == rgVulkanInputType::Hlsl)
                    {
                        hlslFiles[stage] = inputFiles[stage];
                        foundGslsHlslFiles = true;
                    }
                    else if (inputFileTypes[stage] == rgVulkanInputType::SpirvTxt)
                    {
                        spvTxtFiles[stage] = inputFiles[stage];
                        foundSpvTxtFiles = true;
                    }
                    else
                    {
                        spvFiles[stage] = inputFiles[stage];
                    }
                }

                // Pre-compile all glsl or hlsl input files to SPIR-V binaries.
                if (foundGslsHlslFiles)
                {
                    status = status && CompileSourceToSpv(config, glslFiles, hlslFiles, spvFiles);
                }

                // Pre-compile all SPIR-V text files to SPIR-V binaries.
                if (foundSpvTxtFiles)
                {
                    status = status && AssembleSpvTxtInputFiles(config, spvTxtFiles, spvFiles);
                }

                // Create a per-device copy of the build configuration.
                Config configPerDevice = config;

                if (m_asics.empty())
                {
                    std::cout << STR_ERR_NO_TARGET_DEVICE_SPECIFIED << std::endl;
                }
                else
                {
                    // Compile per-device.
                    for (const std::string& target : m_asics)
                    {
                        // Compile for the specific target.
                        configPerDevice.m_ASICs.clear();
                        configPerDevice.m_ASICs.push_back(target);

                        // Now, compile the SPIR-V binaries.
                        if (status)
                        {
                            // Perform back-end compilation from a SPIR-V binary to ISA disassembly & statistics.
                            CompileSpvToIsaForDevice(configPerDevice, spvFiles, target);
                        }
                    }

                    // *****************************
                    // Post-process for all devices.
                    // *****************************

                    // Convert ISA text to CSV if required.
                    if (status && configPerDevice.m_isParsedISARequired)
                    {
                        status = ParseIsaFilesToCSV(true);
                    }

                    // Analyze live registers if requested.
                    if (status && !configPerDevice.m_LiveRegisterAnalysisFile.empty())
                    {
                        status = PerformLiveRegAnalysis(config);
                    }

                    // Generate CFG if requested.
                    if (status && (!configPerDevice.m_blockCFGFile.empty() || !configPerDevice.m_instCFGFile.empty()))
                    {
                        status = ExtractCFG(config);
                    }

                }
            }
        }
    }
}

bool kcCLICommanderVulkan::RunPostCompileSteps(const Config& config)
{
    bool ret = false;
    if (!config.m_sessionMetadataFile.empty())
    {
        ret = GenerateSessionMetadata(config);
        if (!ret)
        {
            rgLog::stdOut << STR_ERR_FAILED_GENERATE_SESSION_METADATA << std::endl;
        }
    }

    DeleteTempFiles();

    return ret;
}

bool kcCLICommanderVulkan::PrintAsicList(const Config& config)
{
    std::set<std::string> allGPUs, matchedDevices;

    bool result = GetSupportedTargets(config, matchedDevices, config.m_printProcessCmdLines);
    assert(result);

    result = result && kcUtils::PrintAsicList(matchedDevices);

    return result;
}

bool kcCLICommanderVulkan::ListEntries(const Config& config, LoggingCallBackFunc_t callback)
{
    std::string preprocText, errMsg;
    bool ret = false;

    if (config.m_InputFiles.size() != 1)
    {
        rgLog::stdOut << STR_ERR_ONE_INPUT_FILE_EXPECTED << std::endl;
    }
    else
    {
        const std::string& filePath = config.m_InputFiles[0];
        bool isHlsl = (config.m_isHlslInput || DetectInputTypeByFileExt(filePath) == rgVulkanInputType::Hlsl);

        if (isHlsl)
        {
            // Preprocess the input file.
            beStatus status = beProgramBuilderVulkan::PreprocessSource(config, config.m_cmplrBinPath, config.m_InputFiles[0], true,
                config.m_printProcessCmdLines, preprocText, errMsg);
            if (status != beStatus_SUCCESS || preprocText.empty())
            {
                rgLog::stdOut << STR_ERR_VULKAN_PREPROCESS_FAILED << config.m_InputFiles[0] << std::endl;
                if (!errMsg.empty())
                {
                    rgLog::stdOut << STR_ERR_VULKAN_GLSLANG_ERROR << std::endl << errMsg << std::endl;
                }
            }
            else
            {
                // Extract the function names from the preprocessed text and dump them to stdout.
                std::vector<std::string> funcNames;
                kcHlslParser parser(preprocText);
                if (parser.GetFuncNames(funcNames))
                {
                    for (const std::string& name : funcNames)
                    {
                        rgLog::stdOut << name << std::endl;
                    }

                    ret = true;
                }
                else
                {
                    rgLog::stdOut << STR_ERR_VULKAN_EXTRACT_HLSL_ENTRIES_FAILED << std::endl;
                }
            }
        }
        else
        {
            rgLog::stdOut << STR_ERR_VULKAN_ENTRY_DETECTION_WRONG_LANGUAGE << std::endl;
        }
    }

    return ret;
}

bool kcCLICommanderVulkan::GetSupportedTargets(const Config& config, std::set<std::string>& targets, bool printCmd /*= false*/)
{
    bool result = false;
    std::string errMsg;
    std::set<std::string> vulkanDevices;

    targets.clear();

    beStatus status = beProgramBuilderVulkan::GetVulkanDriverTargetGPUs(config.m_loaderDebug, config.m_icdFile, vulkanDevices, printCmd, errMsg);
    result = (status == beStatus_SUCCESS);

    if (result && !vulkanDevices.empty())
    {
        std::vector<GDT_GfxCardInfo> cardList;
        std::set<std::string> knownArchNames;

        // Get the list of known GPU architectures from DeviceInfo.
        if ((result = beUtils::GetAllGraphicsCards(cardList, knownArchNames, true)) == true)
        {
            // Filter the Vulkan devices: keep only those devices that are present in the DeviceInfo.
            for (auto it = vulkanDevices.begin(); it != vulkanDevices.end(); )
            {
                // Some device names are returned by Vulkan driver in non-standard form.
                // Try looking for corrected name in the map. If found, replace the device name
                // in the "vulkanDevices" set with the corrected name.
                auto correctedName = std::find_if(PAL_DEVICE_NAME_MAPPING.cbegin(), PAL_DEVICE_NAME_MAPPING.cend(),
                    [&](const std::pair<std::string, std::string>& device) { return (device.first == *it); });

                const std::string& device = (correctedName == PAL_DEVICE_NAME_MAPPING.end() ? *it : correctedName->second);

                if (correctedName != PAL_DEVICE_NAME_MAPPING.end())
                {
                    vulkanDevices.insert(correctedName->second);
                }

                it = (knownArchNames.find(device) == knownArchNames.end() || correctedName != PAL_DEVICE_NAME_MAPPING.end() ?
                    vulkanDevices.erase(it) : ++it);
            }
        }
    }
    else
    {
        rgLog::stdOut << STR_ERR_VULKAN_FAILED_GET_TARGETS << std::endl;
        if (!errMsg.empty())
        {
            rgLog::stdOut << STR_ERR_VULKAN_BACKEND_ERROR << std::endl << errMsg << std::endl;
        }
    }

    targets = vulkanDevices;
    result = !targets.empty();

    return result;
}

bool kcCLICommanderVulkan::GenerateVulkanVersionInfo(const Config& config, const std::string& fileName, bool printCmd /*= false*/)
{
    std::set<std::string> targets;

    // Get the list of supported GPUs for current mode.
    bool result = GetSupportedTargets(config, targets, printCmd);
    assert(result);

    // Add the list of supported GPUs to the Version Info file.
    result = result && kcXmlWriter::AddVersionInfoGPUList(RgaMode::Mode_Vulkan, targets, fileName);
    assert(result);

    return result;
}

bool kcCLICommanderVulkan::GenerateSystemVersionInfo(const Config& config, const std::string& fileName, bool printCmd)
{
    bool result = false;
    std::vector<beVkPhysAdapterInfo> adapterInfo;
    std::string errMsg;

    beStatus status = beProgramBuilderVulkan::GetPhysicalGPUs(config.m_icdFile, adapterInfo, printCmd, errMsg);
    if (status == beStatus_SUCCESS)
    {
        result = kcXmlWriter::AddVersionInfoSystemData(adapterInfo, fileName);
        assert(result);
    }

    return result;
}

bool kcCLICommanderVulkan::InitRequestedAsicListVulkan(const Config& config)
{
    bool result = false;
    bool usePhysicalAdapter = (config.m_ASICs.empty());

    std::set<std::string> vulkanDevices, matchedTargets;

    // Get the list of supported target devices.
    result = GetSupportedTargets(config, vulkanDevices, config.m_printProcessCmdLines);
    if (!result)
    {
        rgLog::stdOut << STR_ERR_CANNOT_EXTRACT_SUPPORTED_DEVICE_LIST << std::endl;
    }
    else
    {
        if (usePhysicalAdapter)
        {
            std::string errMsg;
            std::vector<beVkPhysAdapterInfo> physAdapters;
            beStatus status = beProgramBuilderVulkan::GetPhysicalGPUs(config.m_icdFile, physAdapters,
                config.m_printProcessCmdLines, errMsg);
            if (status == beStatus_SUCCESS)
            {
                m_physAdapterName = physAdapters[0].name;
            }
            else
            {
                rgLog::stdOut << STR_ERR_VULKAN_FAILED_GET_ADAPTERS << std::endl;
                result = false;
            }
        }
        else
        {
            if ((result = InitRequestedAsicList(config.m_ASICs, config.m_mode, vulkanDevices, matchedTargets, false)) == true)
            {
                m_asics = matchedTargets;
            }
        }
    }

    if (m_asics.empty() && !usePhysicalAdapter)
    {
        rgLog::stdOut << STR_ERR_VULKAN_DEVICES_NOT_SUPPORTED <<
            [&]() {std::string s; for (auto& d : config.m_ASICs) s += (d + " "); return s; }() << std::endl;
    }

    return result;
}

bool kcCLICommanderVulkan::CompileSourceToSpv(const Config& conf, const beVkPipelineFiles& glslFiles,
    const beVkPipelineFiles& hlslFiles, beVkPipelineFiles& outSpvFiles)
{
    bool result = true;
    std::string errMsg;
    beVkPipelineFiles srcFiles;

    for (int stage = 0; stage < bePipelineStage::Count; stage++)
    {
        bool isHlsl = !hlslFiles[stage].empty();
        srcFiles[stage] = (isHlsl ? hlslFiles[stage] : glslFiles[stage]);
        if (!srcFiles[stage].empty())
        {
            std::string msg = KA_CLI_STR_PRECOMPILING_A + STR_PIPELINE_STAGE_FULL_NAMES[stage] + KA_CLI_STR_PRECOMPILING_B;
            msg += ("(" + srcFiles[stage] + ")");
            msg += KA_CLI_STR_PRECOMPILING_C;

            // Construct a name for temporary spv file.
            beStatus status = beStatus_SUCCESS;
            if (kcUtils::ConstructOutFileName("", STR_VULKAN_STAGE_FILE_SUFFIXES_DEFAULT[stage], STR_VULKAN_FILE_SUFFIX_ALL_DEVICES,
                STR_VULKAN_SPIRV_FILE_EXT, outSpvFiles[stage]))
            {
                m_tempFiles.push_back(outSpvFiles[stage]);
            }
            else
            {
                status = beStatus_Vulkan_ConstructOutFileNameFailed;
            }

            // Notify the user about the front-end compilation.
            msg += (" (" + outSpvFiles[stage] + ")");
            LogPreStep(msg);

            if (status == beStatus_SUCCESS)
            {
                bePipelineStage pipelineStage = static_cast<bePipelineStage>(stage);
                status = beProgramBuilderVulkan::CompileSrcToSpirvBinary(conf, srcFiles[stage],
                    outSpvFiles[stage], pipelineStage, isHlsl, errMsg);
            }

            // Check if output spv file has not been generated for some reason.
            if (status == beStatus_SUCCESS && !kcUtils::FileNotEmpty(outSpvFiles[stage]))
            {
                status = beStatus_Vulkan_FrontendCompileFailed;
            }

            assert(status == beStatus_SUCCESS);
            LogResult(status == beStatus_SUCCESS);

            if (status != beStatus_SUCCESS)
            {
                LogErrorStatus(status, errMsg);
                result = false;
                break;
            }
        }
    }

    // Store the input file names to the output metadata.
    if (result)
    {
        StoreInputFilesToOutputMD(srcFiles);
    }

    return result;
}

void kcCLICommanderVulkan::CompileSpvToIsa(const Config& conf, const beVkPipelineFiles& spvFiles)
{
    if (m_asics.empty())
    {
        // Compile for physical adapter installed on the system.
        CompileSpvToIsaForDevice(conf, spvFiles, m_physAdapterName, true);
    }
    else
    {
        // Compile for selected target devices.
        for (const std::string& device : m_asics)
        {
            CompileSpvToIsaForDevice(conf, spvFiles, device, false);
        }
    }
}

void kcCLICommanderVulkan::CompileSpvToIsaForDevice(const Config& config, const beVkPipelineFiles& spvFiles,
    const std::string& device, bool isPhysAdapter)
{
    const std::string& deviceSuffix = (isPhysAdapter ? "" : device);

    LogPreStep(KA_CLI_STR_COMPILING, device);

    beVkPipelineFiles  isaFiles, statsFiles;

    bool result = false;
    std::string isaFileBaseName = config.m_ISAFile;
    std::string statsFileBaseName = config.m_AnalysisFile;
    std::string binFileBaseName = config.m_BinaryOutputFile;
    std::string binFileName, validationFileName, errMsg;
    beStatus status = beStatus_General_FAILED;

    // Construct names for output files.
    result = ConstructVkOutputFileNames(binFileBaseName, isaFileBaseName, statsFileBaseName,
        deviceSuffix, spvFiles, binFileName, isaFiles, statsFiles);

    if (result && !config.m_vulkanValidation.empty())
    {
        // Construct name for Vulkan validation info output file.
        validationFileName = kcUtils::ConstructTempFileName(STR_VULKAN_TEMP_VALIDATION_INFO_FILE_NAME,
            STR_VULKAN_VALIDATION_INFO_FILE_EXT);
        result = !validationFileName.empty();
    }

    // A flag that indicates if we fell back to vk-spv-offline mode.
    bool isVkOffline = false;

    if (result)
    {
        // If the Vulkan driver uses non-standard name for this device, convert it back to the driver format.
        auto correctedDevice = std::find_if(PAL_DEVICE_NAME_MAPPING.cbegin(), PAL_DEVICE_NAME_MAPPING.cend(),
            [&](const std::pair<std::string, std::string>& d) { return (d.second == device); });

        const std::string& vulkanDevice = (correctedDevice == PAL_DEVICE_NAME_MAPPING.end() ? device : correctedDevice->first);

        // Remove ISA output files if they exist before attempting to compile.
        for (const std::string& outFileName : isaFiles)
        {
            if (!outFileName.empty())
            {
                kcUtils::DeleteFile(outFileName);
            }
        }

        // Remove stats files if they exist before attempting to compile.
        for (const std::string& outFileName : statsFiles)
        {
            if (!outFileName.empty())
            {
                kcUtils::DeleteFile(outFileName);
            }
        }

        // Perform the compilation.
        status = beProgramBuilderVulkan::CompileSpirv(config.m_loaderDebug, spvFiles, isaFiles, statsFiles, binFileName, config.m_pso,
            config.m_icdFile, validationFileName, config.m_vulkanValidation,
            (isPhysAdapter ? "" : vulkanDevice), config.m_printProcessCmdLines, errMsg);

        if (status != beStatus_SUCCESS)
        {
            // Report the backend error.
            std::cout << std::endl << errMsg << std::endl;
            errMsg.clear();

            // Warn the user.
            std::cout << STR_WRN_VULKAN_FALLBACK_TO_VK_OFFLIINE_MODE << std::endl << std::endl;
            std::cout << KA_CLI_STR_FALLING_BACK_TO_OFFLINE_MODE << std::endl;
            isVkOffline = true;

            // Adjust the config file to the vk-spv-offline mode, and target only the current device.
            Config vkOfflineConfig = config;
            vkOfflineConfig.m_mode = Mode_Vk_Offline_Spv;
            vkOfflineConfig.m_InputFiles.clear();
            vkOfflineConfig.m_ASICs.clear();
            vkOfflineConfig.m_ASICs.push_back(device);

            // Adjust the configuration structure to reference the compiled SPIR-V binaries.
            vkOfflineConfig.m_VertexShader = spvFiles[bePipelineStage::Vertex];
            vkOfflineConfig.m_TessControlShader = spvFiles[bePipelineStage::TessellationControl];
            vkOfflineConfig.m_TessEvaluationShader = spvFiles[bePipelineStage::TessellationEvaluation];
            vkOfflineConfig.m_GeometryShader = spvFiles[bePipelineStage::Geometry];
            vkOfflineConfig.m_FragmentShader = spvFiles[bePipelineStage::Fragment];
            vkOfflineConfig.m_ComputeShader = spvFiles[bePipelineStage::Compute];

            // Fallback to using vk-spv-offline.
            kcCLICommanderVkOffline commander;
            commander.RunCompileCommands(vkOfflineConfig, loggingCallback);

            // Verify that the compilation succeeded.
            bool isBuildSuccess = true;
            for (int stage = 0; stage < bePipelineStage::Count; stage++)
            {
                if (!spvFiles[stage].empty())
                {
                    if ((!vkOfflineConfig.m_ISAFile.empty() && !kcUtils::FileNotEmpty(isaFiles[stage])) ||
                        (!vkOfflineConfig.m_AnalysisFile.empty() && !kcUtils::FileNotEmpty(statsFiles[stage])))
                    {
                        isBuildSuccess = false;
                        break;
                    }
                }
            }

            // If the compilation succeeded - mark this as a success.
            if (isBuildSuccess)
            {
                status = beStatus_SUCCESS;
            }
        }

        if (!isVkOffline && status == beStatus_SUCCESS)
        {
            // Notify the user about shader merge if happened.
            if (kcUtils::IsNaviTarget(device) || kcUtils::IsVegaTarget(device))
            {
                bool isFirstMsg = true;
                if (!spvFiles[bePipelineStage::Geometry].empty() && spvFiles[bePipelineStage::TessellationEvaluation].empty())
                {
                    if (beUtils::IsFilesIdentical(isaFiles[bePipelineStage::Vertex], isaFiles[bePipelineStage::Geometry]))
                    {
                        if (isFirstMsg)
                        {
                            std::cout << std::endl;
                            isFirstMsg = false;
                        }
                        std::cout << STR_INFO_VULKAN_GEOM_VERT_MERGED << std::endl;
                     }
                    isFirstMsg = true;
                }
                else if (!spvFiles[bePipelineStage::Geometry].empty() && !spvFiles[bePipelineStage::TessellationEvaluation].empty())
                {
                    if (beUtils::IsFilesIdentical(isaFiles[bePipelineStage::TessellationEvaluation], isaFiles[bePipelineStage::Geometry]))
                    {
                        if (isFirstMsg)
                        {
                            std::cout << std::endl;
                            isFirstMsg = false;
                        }
                        std::cout << STR_INFO_VULKAN_GEOM_TESE_MERGED << std::endl;
                    }
                }

                if (!spvFiles[bePipelineStage::TessellationControl].empty())
                {
                    if (beUtils::IsFilesIdentical(isaFiles[bePipelineStage::Vertex], isaFiles[bePipelineStage::TessellationControl]))
                    {
                        if (isFirstMsg)
                        {
                            std::cout << std::endl;
                            isFirstMsg = false;
                        }
                        std::cout << STR_INFO_VULKAN_TESC_VERT_MERGED << std::endl;
                    }
                }
            }


            status = ConvertStats(isaFiles, statsFiles, config, device);
        }

        if (status == beStatus_SUCCESS)
        {
            StoreOutputFilesToOutputMD(device, spvFiles, isaFiles, statsFiles);
        }

        // If temporary ISA files are used, add their paths to the list of temporary files.
        if (!isVkOffline && isaFileBaseName.empty())
        {
            std::copy_if(isaFiles.cbegin(), isaFiles.cend(), std::back_inserter(m_tempFiles),
                [&](const std::string& s) { return !s.empty(); });
        }
    }
    else
    {
        status = beStatus_ConstructIsaFileNameFailed;
    }

    // Only print the status messages if we did not fall back
    // to vk-spv-offline mode, since otherwise logging happens in that mode.
    if (!isVkOffline)
    {
        LogResult(status == beStatus_SUCCESS);
        LogErrorStatus(status, errMsg);
    }
}

bool kcCLICommanderVulkan::AssembleSpv(const Config& conf)
{
    std::string inputSpvTxtFile, errMsg;
    if (conf.m_InputFiles.size() == 1)
    {
        inputSpvTxtFile = conf.m_InputFiles[0];
    }
    else if (conf.m_InputFiles.empty())
    {
        // User did not provide output path.
        rgLog::stdOut << STR_ERR_ASSEMBLE_NO_OUTPUT_FILE << std::endl;
    }
    else
    {
        // User provided too many arguments.
        rgLog::stdOut << STR_ERR_INVALID_NUM_ARGS << std::endl;
    }

    bool result = !inputSpvTxtFile.empty() && !conf.m_spvBin.empty();
    if (result)
    {
        LogPreStep(KA_CLI_STR_ASSEMBLING + inputSpvTxtFile);

        beStatus status = beProgramBuilderVulkan::AssembleSpv(conf.m_cmplrBinPath, inputSpvTxtFile, conf.m_spvBin,
            conf.m_printProcessCmdLines, errMsg);

        result = (status == beStatus_SUCCESS);
        LogResult(result);

        if (status != beStatus_SUCCESS)
        {
            rgLog::stdOut << STR_ERR_VULKAN_SPV_ASM_FAILED << inputSpvTxtFile << std::endl;

            if (!errMsg.empty())
            {
                rgLog::stdOut << STR_ERR_VULKAN_SPV_ASM_ERR_MSG << std::endl << errMsg << std::endl;
            }
        }
    }

    return result;
}

bool kcCLICommanderVulkan::DisassembleSpv(const Config& conf)
{
    std::string errMsg;
    std::string inputSpvFile, outputSpvDisFile;

    // If input file is empty, consider the argument of "--disassemble-spv" option as an input file and dump
    // disassembly text to the stdout.
    if (conf.m_InputFiles.empty())
    {
        inputSpvFile = conf.m_spvTxt;
    }
    else
    {
        inputSpvFile = conf.m_InputFiles[0];
        outputSpvDisFile = conf.m_spvTxt;
    }

    bool result = !inputSpvFile.empty();
    if (result)
    {
        LogPreStep(KA_CLI_STR_DISASSEMBLING + inputSpvFile);

        beStatus status = beProgramBuilderVulkan::DisassembleSpv(conf.m_cmplrBinPath, inputSpvFile, outputSpvDisFile,
            conf.m_printProcessCmdLines, errMsg);

        result = (status == beStatus_SUCCESS);
        LogResult(result);

        if (status != beStatus_SUCCESS)
        {
            rgLog::stdOut << STR_ERR_VULKAN_SPV_DISASM_FAILED << inputSpvFile << std::endl;

            if (!errMsg.empty())
            {
                rgLog::stdOut << STR_ERR_VULKAN_SPV_DIS_ERR_MSG << std::endl << errMsg << std::endl;
            }
        }
    }

    return (result);
}

// Dump the SPIR-V info to the specified output stream (file or stdout).
static bool DumpSpvInfo(const std::multimap<spv::ExecutionModel, std::string>& entries, std::ostream& outStream, const std::string& title)
{
    bool result = true;

    rgLog::stdOut << title << std::endl << std::endl;

    for (auto execModel = entries.cbegin(); execModel != entries.cend(); execModel = entries.upper_bound(execModel->first))
    {
        auto stage = PIPELINE_STAGE_FOR_SPV_EXEC_MODEL.find(execModel->first);
        assert(stage != PIPELINE_STAGE_FOR_SPV_EXEC_MODEL.cend());
        if (stage != PIPELINE_STAGE_FOR_SPV_EXEC_MODEL.cend())
        {
            outStream << STR_PIPELINE_STAGE_NAMES[stage->second] << ":";
            auto stageEntries = entries.equal_range(execModel->first);
            for (auto entry = stageEntries.first; entry != stageEntries.second; entry++)
            {
                outStream << " \"" << entry->second << "\"";
            }
            outStream << std::endl;
        }
        else
        {
            rgLog::stdOut << STR_ERR_VULKAN_NO_PIPELINE_STAGE_FOR_SPV_EXEC_MODEL << std::endl;
            result = false;
            break;
        }
    }

    return result;
}

bool kcCLICommanderVulkan::ParseSpv(const Config& conf)
{
    std::vector<spirv_cross::EntryPoint> entries;
    std::string inputSpvFile, outputInfoFile;
    bool result = false;

    // If input file is empty, consider the argument of "--parse-spv" option as an input file and dump
    // disassembly text to the stdout.
    if (conf.m_InputFiles.empty())
    {
        inputSpvFile = conf.m_parsedSpv;
        result = true;
    }
    else if (conf.m_InputFiles.size() == 1)
    {
        inputSpvFile = conf.m_InputFiles[0];
        outputInfoFile = conf.m_parsedSpv;
        result = true;
    }
    else
    {
        rgLog::stdOut << STR_ERR_ONE_INPUT_FILE_EXPECTED << std::endl;
    }

    if (result)
    {
        if ((result = IsSpvBinFile(inputSpvFile)) == true)
        {
            rgLog::stdOut << KA_CLI_STR_PARSING_SPV << inputSpvFile << "... ";
            result = ExtractSpvEntries(inputSpvFile, entries);
            rgLog::stdOut << (result ? KA_CLI_STR_STATUS_SUCCESS : KA_CLI_STR_STATUS_FAILURE) << std::endl << std::endl;
        }
        else
        {
            rgLog::stdOut << STR_ERR_VULKAN_FILE_IS_NOT_SPV_BINARY << inputSpvFile << std::endl;
        }
    }

    if (result)
    {
        std::multimap<spv::ExecutionModel, std::string> entryMap;

        // Classify entries by shader stage (execution model).
        for (const spirv_cross::EntryPoint& entry : entries)
        {
            entryMap.insert({ entry.execution_model, entry.name });
        }

        if (outputInfoFile.empty())
        {
            result = DumpSpvInfo(entryMap, std::cout, KC_STR_SPIRV_INFO);
        }
        else
        {
            std::ofstream outFile(outputInfoFile);
            if (outFile.good())
            {
                const std::string title = KC_STR_SPIRV_INFO_SAVED_TO_FILE + outputInfoFile;
                result = DumpSpvInfo(entryMap, outFile, title);
            }
            else
            {
                std::cerr << STR_ERR_CANNOT_OPEN_FILE_FOR_WRITE_A << outputInfoFile << STR_ERR_CANNOT_OPEN_FILE_FOR_WRITE_B << std::endl;
                result = false;
            }
        }
    }

    return result;
}

bool kcCLICommanderVulkan::AssembleSpvTxtInputFiles(const Config& conf, const beVkPipelineFiles& spvTxtFiles, beVkPipelineFiles& outSpvFiles)
{
    bool result = true;
    std::string errMsg;

    for (int stage = 0; stage < bePipelineStage::Count; stage++)
    {
        if (!spvTxtFiles[stage].empty())
        {
            LogPreStep(KA_CLI_STR_PRECOMPILING_A + STR_PIPELINE_STAGE_NAMES[stage] + KA_CLI_STR_PRECOMPILING_B);

            // Construct a name for temporary spv file.
            result = kcUtils::ConstructOutFileName("", STR_VULKAN_STAGE_FILE_SUFFIXES_DEFAULT[stage], "", STR_VULKAN_SPIRV_FILE_EXT, outSpvFiles[stage]);

            if (result)
            {
                beStatus status = beProgramBuilderVulkan::AssembleSpv(conf.m_cmplrBinPath, spvTxtFiles[stage],
                    outSpvFiles[stage], conf.m_printProcessCmdLines, errMsg);
                LogResult(status == beStatus_SUCCESS);

                if (status != beStatus_SUCCESS)
                {
                    result = false;
                    rgLog::stdOut << STR_ERR_VULKAN_SPV_ASM_FAILED << spvTxtFiles[stage] << std::endl;

                    if (!errMsg.empty())
                    {
                        rgLog::stdOut << STR_ERR_VULKAN_SPV_ASM_ERR_MSG << std::endl << errMsg << std::endl;
                        errMsg.clear();
                    }
                }

                m_tempFiles.push_back(outSpvFiles[stage]);
            }
            else
            {
                break;
            }
        }
    }

    // Store the input file names to the output metadata.
    if (result)
    {
        StoreInputFilesToOutputMD(spvTxtFiles);
    }

    return result;
}

bool kcCLICommanderVulkan::GenerateSessionMetadata(const Config& config) const
{
    return kcXmlWriter::GenerateVulkanSessionMetadataFile(config.m_sessionMetadataFile, m_outputMD);
}

bool kcCLICommanderVulkan::ParseIsaFilesToCSV(bool lineNumbers)
{
    bool ret = true;

    // Step through existing output items to determine which files to generate CSV ISA for.
    for (auto& outputMDItem : m_outputMD)
    {
        const std::string& deviceString = outputMDItem.first;
        rgVkOutputMetadata& metadata = outputMDItem.second;

        for (rgOutputFiles& outputFile : metadata)
        {
            if (!outputFile.m_inputFile.empty())
            {
                std::string isa, parsedIsa, parsedIsaFileName;
                bool status = kcUtils::ReadTextFile(outputFile.m_isaFile, isa, nullptr);

                if (status)
                {
                    // Convert the ISA text to CSV format.
                    if ((status = GetParsedIsaCSVText(isa, deviceString, lineNumbers, parsedIsa)) == true)
                    {
                        status = (kcUtils::GetParsedISAFileName(outputFile.m_isaFile, parsedIsaFileName) == beKA::beStatus_SUCCESS);
                        if (status)
                        {
                            // Attempt to write the ISA CSV to disk.
                            status = (WriteISAToFile(parsedIsaFileName, parsedIsa) == beKA::beStatus_SUCCESS);
                            if (status)
                            {
                                // Update the session metadata output to include the path to the ISA CSV.
                                outputFile.m_isaCsvFile = parsedIsaFileName;
                            }
                        }
                    }

                    if (!status)
                    {
                        rgLog::stdOut << STR_ERR_FAILED_ISA_TO_CSV_FILE_NAME << outputFile.m_isaFile << std::endl;
                    }
                }
                ret &= status;
            }
        }
    }

    return ret;
}

bool kcCLICommanderVulkan::PerformLiveRegAnalysis(const Config& conf) const
{
    bool  ret = true;

    for (const auto& deviceMDNode : m_outputMD)
    {
        const std::string& device = deviceMDNode.first;
        const std::string& deviceSuffix = (conf.m_ASICs.empty() && !m_physAdapterName.empty() ? "" : device);
        const rgVkOutputMetadata& deviceMD = deviceMDNode.second;

        std::cout << STR_INFO_PERFORMING_LIVEREG_ANALYSIS_A << device << "... " << std::endl;

        for (int stage = 0; stage < bePipelineStage::Count && ret; stage++)
        {
            const rgOutputFiles& stageMD = deviceMD[stage];

            if (!stageMD.m_inputFile.empty())
            {
                std::string outFileName;
                gtString gOutFileName, gIsaFileName;

                // Construct a name for the livereg output file.
                ret = kcUtils::ConstructOutFileName(conf.m_LiveRegisterAnalysisFile, STR_VULKAN_STAGE_FILE_SUFFIXES_DEFAULT[stage],
                    deviceSuffix, KC_STR_DEFAULT_LIVEREG_EXT, outFileName);

                if (ret && !outFileName.empty())
                {
                    gOutFileName << outFileName.c_str();
                    gIsaFileName << stageMD.m_isaFile.c_str();

                    kcUtils::PerformLiveRegisterAnalysis(gIsaFileName, gOutFileName, m_LogCallback, conf.m_printProcessCmdLines);
                    ret = beUtils::IsFilePresent(outFileName);
                }
                else
                {
                    rgLog::stdOut << STR_ERR_FAILED_CREATE_OUTPUT_FILE_NAME << std::endl;
                }
            }
        }
    }

    LogResult(ret);

    return ret;
}

bool kcCLICommanderVulkan::ExtractCFG(const Config& conf) const
{
    bool  ret = true;

    for (const auto& deviceMDNode : m_outputMD)
    {
        const std::string& device = deviceMDNode.first;
        const std::string& deviceSuffix = (conf.m_ASICs.empty() && !m_physAdapterName.empty() ? "" : device);
        const rgVkOutputMetadata& deviceMD = deviceMDNode.second;
        bool perInstCfg = (!conf.m_instCFGFile.empty());

        std::cout << (perInstCfg ? STR_INFO_CONSTRUCTING_INSTRUCTION_CFG_A:
            STR_INFO_CONSTRUCTING_BLOCK_CFG_A) << device << "..." << std::endl;

        for (int stage = 0; stage < bePipelineStage::Count && ret; stage++)
        {
            const rgOutputFiles& stageMD = deviceMD[stage];

            if (!stageMD.m_inputFile.empty())
            {
                std::string outFileName;
                gtString gOutFileName, gIsaFileName;

                // Construct a name for the CFG output file.
                ret = kcUtils::ConstructOutFileName((perInstCfg ? conf.m_instCFGFile : conf.m_blockCFGFile),
                    STR_VULKAN_STAGE_FILE_SUFFIXES_DEFAULT[stage],
                    deviceSuffix, KC_STR_DEFAULT_CFG_EXT, outFileName);

                if (ret && !outFileName.empty())
                {
                    gOutFileName << outFileName.c_str();
                    gIsaFileName << stageMD.m_isaFile.c_str();

                    kcUtils::GenerateControlFlowGraph(gIsaFileName, gOutFileName, m_LogCallback, perInstCfg, conf.m_printProcessCmdLines);
                    ret = beUtils::IsFilePresent(outFileName);
                }
                else
                {
                    rgLog::stdOut << STR_ERR_FAILED_CREATE_OUTPUT_FILE_NAME << std::endl;
                }
            }
        }
    }

    LogResult(ret);

    return ret;
}

void kcCLICommanderVulkan::StoreInputFilesToOutputMD(const beVkPipelineFiles& inputFiles)
{
    for (const std::string& device : m_asics)
    {
        // If the output metadata for "device" does not exist, create a new node.
        // Otherwise, just add new input files to the existing node.
        if (m_outputMD.find(device) == m_outputMD.end())
        {
            rgVkOutputMetadata outMD;

            for (int stage = 0; stage < bePipelineStage::Count; stage++)
            {
                rgOutputFiles outFiles;
                outFiles.m_device = device;
                outFiles.m_entryType = VULKAN_OGL_STAGE_ENTRY_TYPES[stage];
                outFiles.m_inputFile = inputFiles[stage];
                outMD[stage] = outFiles;
            }

            m_outputMD[device] = outMD;
        }
        else
        {
            for (int stage = 0; stage < bePipelineStage::Count; stage++)
            {
                rgOutputFiles& outFiles = m_outputMD[device][stage];
                if (!inputFiles[stage].empty())
                {
                    assert(outFiles.m_inputFile.empty());
                    outFiles.m_inputFile = inputFiles[stage];
                }
            }
        }
    }
}

void kcCLICommanderVulkan::StoreOutputFilesToOutputMD(const std::string& device, const beVkPipelineFiles& spvFiles,
    const beVkPipelineFiles& isaFiles, const beVkPipelineFiles& statsFiles)
{
    // Check if the output Metadata for this device already exists.
    // It exists if some of shader files are GLSL/HLSL or SPIR-V text files. In that case,
    // the metadata has been created during pre-compiling the shaders to SPIR-V binary format
    bool deviceMDExists = (m_outputMD.find(device) != m_outputMD.end());
    if (!deviceMDExists)
    {
        rgVkOutputMetadata md;
        rgOutputFiles outFiles(device);
        md.fill(outFiles);
        m_outputMD[device] = md;
    }

    rgVkOutputMetadata& deviceMD = m_outputMD[device];

    for (int stage = 0; stage < bePipelineStage::Count; stage++)
    {
        if (!spvFiles[stage].empty())
        {
            // If the "input file" in the metadata for this stage is non-empty, keep it there (it's the path to the
            // GLSL/HLSL/spv-text file that was pre-compiled to a temporary SPIR-V binary file).
            // If it's empty, store the spv file path as an "input file".
            rgOutputFiles& stageMD = deviceMD[stage];
            if (stageMD.m_inputFile.empty())
            {
                stageMD.m_inputFile = spvFiles[stage];
                stageMD.m_entryType = VULKAN_OGL_STAGE_ENTRY_TYPES[stage];
                stageMD.m_device = device;
            }
            stageMD.m_isaFile = isaFiles[stage];
            stageMD.m_statFile = statsFiles[stage];
        }
    }
}

void kcCLICommanderVulkan::DeleteTempFiles()
{
    for (const std::string& tmpFile : m_tempFiles)
    {
        kcUtils::DeleteFile(tmpFile);
    }

    m_tempFiles.clear();
}
