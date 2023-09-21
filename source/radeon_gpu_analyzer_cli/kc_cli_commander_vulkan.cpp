//======================================================================
// Copyright 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

// C++.
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
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/vulkan/tools/include/spirv_cross/spirv_cross.hpp"
#ifdef _WIN32
#pragma warning(pop)
#endif

// Shared.
#include "common/rga_cli_defs.h"
#include "common/rg_log.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_string_constants.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_vulkan.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_vk_offline.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_vulkan_util.h"

using namespace beKA;

// Constants.

// Magic number for SPIR-V binary files (SPIR-V Spec, par. 3.1)
static const uint32_t  SPV_BINARY_MAGIC_NUMBER = spv::MagicNumber;

// Extensions of different input/output file types.
static const std::string  kStrVulkanSpirvFileExtension          = "spv";
static const std::string  kStrVulkanSpirvTextFileExtension      = "spvasm";
static const std::string  kStrVulkanBinaryFileExtension         = "bin";
static const std::string  kStrVulkanIsaFileExtension            = "isa";
static const std::string  kStrVulkanStatsFileExtension          = "csv";
static const std::string  kStrVulkanValidationInfoFileExtension = "txt";
static const std::string  kStrVulkanHlslFileExtension           = "hlsl";

// Default name for temporary files.
static const std::string  kStrVulkanTempValidationInfoFilename  = "rga-vk-validation-info";

// Temp file suffix for all devices.
static const std::string  kStrVulkanFileSuffixAllDevices        = "all-devices";

// The name of the VK_LOADER_DEBUG environment variable.
static const std::string  kStrVulkanVkLoaderDebugEnvVarName     = "VK_LOADER_DEBUG";

// Suffixes for stage-specific output files - cont.
// We need to keep these separate from the default ones
// since the default container is being accessed using the
// [] operator and are the items are assumed to be in order.
static const std::vector<std::string>
kVulkanStageFileSuffix1 =
{
    "vs",
    "fs",
    "glsl"
};

// Error messages.
static const char* kStrErrorVulkanFailedToConvertStats = "Error: failed to convert Vulkan driver statistics to RGA format.";
static const char* kStrErrorVulkanAssembleNoOutputFile = "Error: output file path for assembled SPIR-V file not provided.";
static const char* kStrErrorVulkanInvalidNumArgs = "Error: invalid number of arguments.";
static const char* kStrErrorVulkanFailedToGetTargets = "Error: failed to get the list of target GPUs.";
static const char* kStrErrorVulkanFailedToGetAdapters = "Error: failed to get the list of compatible display adapters installed on the system.";
static const char* kStrErrorVulkanDevicesNotSupported = "Error: specified GPUs are not supported: ";
static const char* kStrErrorVulkanFailedToAssembleSpirv = "Error: failed to assemble SPIR-V text file: ";
static const char* kStrErrorVulkanFailedToDisassembleSpirv = "Error: failed to disassemble SPIR-V binary file: ";
static const char* kStrErrorVulkanSpirvAssemblerErrorMessage = "SPIR-V assembler error message:";
static const char* kStrErrorVulkanSpirvDisassemblerErrorMessage = "SPIR-V disassembler error message:";
static const char* kStrErrorVulkanFailedToLaunchBackend = "Error: failed to launch the Vulkan Backend process.";
static const char* kStrErrorVulkanBackendFailure = "Error: Vulkan backend compilation failed.";
static const char* kStrErrorVulkanBackendErrorMessage = "Vulkan backend compiler error message:";
static const char* kStrErrorVulkanFailedToLaunchGlslang = "Error: failed to launch the Glslang compiler process.";
static const char* kStrErrorVulkanFrontendCompilationFailed = "Error: Vulkan front-end compilation failed.";
static const char* kStrErrorVulkanPrepprocessFailed = "Error: failed to preprocess source file: ";
static const char* kStrErrorVulkanFailedToExtractHlslEntries = "Error: failed to extract HLSL function names.";
static const char* kStrErrorVulkanEntryDetectionWrongLanguage = "Error: entry point extraction only supported for HLSL. Please use \"--hlsl\" option to specify the input file explicitly.";
static const char* kStrErrorVulkanGlslangErrorMessage = "Glslang compiler error message:";
static const char* kStrErrorVulkanFailedToDetectInputFileByExtension1 = "Error: cannot detect type of input file(s) by extension. Use --hlsl, --spv or --spv-txt option to specify the Vulkan input type.";
static const char* kStrErrorVulkanNoPipelineStageForSpirvExecModel = "Error: failed to find pipeline stage for SPIR-V Execution Model.";
static const char* kStrErrorVulkanFileIsNotSpirvBinary = "Error: specified file is not a SPIR-V binary: ";
static const char* kStrErrorVulkanGpsoOptionNotSupported = "Error: invalid option --gpso - not supported by this mode. Did you mean --pso?";
static const char* kStrErrorVulkanNoTargetSpecified = "Error: no target device specified. Use the -c or --asic options to specify the target device. For the list of all supported device use the -l option.";
static const char* kStrErrorVulkanFailedToDetectInputFileByExtension2 = "Error: cannot detect type of input file(s) by extension: same extension for all input files is expected."
" Use --hlsl, --spv or --spv-txt option to specify the Vulkan input type.";

// Warnings.
static const char* kStrWarningVulkanFailedToSetEnvVar1 = "Warning: failed to set the ";
static const char* kStrWarningVulkanFailedToSetEnvVar2 = "environment variable.";
static const char* kStrWarningVulkanFallbackToVkOfflineMode = "Warning: falling back to building using Vulkan offline mode (-s vk-spv-offline). "
"The generated ISA disassembly and HW resource usage information might be inaccurate. To get the most accurate results, adjust the pipeline state to match the shaders and rebuild.";

// Info messages.
static const char* kStrInfoVulkanAssemblingSpirv = "Assembling SPIR-V text file: ";
static const char* kStrInfoVulkanDisassemblingSpirv = "Disassembling SPIR-V binary: ";
static const char* kStrInfoVulkanPrecompiling1 = "Pre-compiling ";
static const char* kStrInfoVulkanPrecompiling2 = " shader file ";
static const char* kStrInfoVulkanPrecompiling3 = " to SPIR-V binary";
static const char* kStrInfoVulkanParsingSpirv = "Parsing SPIR-V binary ";
static const char* kStrVulkanSpirvInfo = "SPIR-V Info:";
static const char* kStrVulkanSpirvInfoSavedToFile = "SPIR-V Info saved to file: ";
static const char* kStrInfoVulkanFallingBackToOfflineMode = "Falling back to vk-spv-offline mode...";

// Map:  SPIR-V execution model --> RGA pipeline stage.
static const std::map<spv::ExecutionModel, BePipelineStage>
PIPELINE_STAGE_FOR_SPV_EXEC_MODEL =
{
    {spv::ExecutionModel::ExecutionModelFragment,               BePipelineStage::kFragment},
    {spv::ExecutionModel::ExecutionModelGeometry,               BePipelineStage::kGeometry},
    {spv::ExecutionModel::ExecutionModelGLCompute,              BePipelineStage::kCompute},
    {spv::ExecutionModel::ExecutionModelTessellationControl,    BePipelineStage::kTessellationControl},
    {spv::ExecutionModel::ExecutionModelTessellationEvaluation, BePipelineStage::kTessellationEvaluation},
    {spv::ExecutionModel::ExecutionModelVertex,                 BePipelineStage::kVertex}
};

// Suffixes for stage-specific output files.
static const std::array<std::string, BePipelineStage::kCount> 
kVulkanStageFileSuffixDefault = 
{
    "vert", 
    "tesc", 
    "tese", 
    "geom", 
    "frag", 
    "comp"
};

// File extensions for GLSL source files - default.
static const std::array<std::string, BePipelineStage::kCount>
kStrGlslFileExtensions = kVulkanStageFileSuffixDefault;

// File extensions for GLSL source files - additional.
static const std::vector<std::string>
kStrGlslFileExtensionsAdditional = kVulkanStageFileSuffix1;

// Pipeline stage names for parsed SPIR-V info.
static const std::array<std::string, BePipelineStage::kCount>
kStrPipelineStageNames = kVulkanStageFileSuffixDefault;

// Output metadata entry types for OpenGL pipeline stages.
static const std::array<RgEntryType, BePipelineStage::kCount> 
kVulkanOglStageEntryTypes = 
{
    RgEntryType::kGlVertex,
    RgEntryType::kGlTessControl,
    RgEntryType::kGlTessEval,
    RgEntryType::kGlGeometry,
    RgEntryType::kGlFragment,
    RgEntryType::kGlCompute
};

// Pipeline stage full names used in text messages.
static const std::array<std::string, BePipelineStage::kCount>
kStrPipelineStageNamesFull =
{
    kStrVertexStage,
    kStrTessellationControlStageName,
    kStrTessellationEvaluationStageName,
    kStrGeometryStageName,
    kStrFragmentStageName,
    kStrComputeStageName
};

// HLSL text parser.
// Used to look through specified HLSL program text and extract required data.
// Note: the HLSL text must be preprocessed for accurate results.
class kcHlslParser
{
public:
    kcHlslParser(const std::string& text) : text_(text), text_size_(text.size()) {}

    // Extract the function names.
    bool GetFuncNames(std::vector<std::string>& names)
    {
        int scope_depth = 0;
        char sym;
        size_t pos = 0;

        while ((pos = Fetch(sym)) != kTextEnd)
        {
            switch (sym)
            {
            case '{': scope_depth++; break;
            case '}': scope_depth--; break;
            case '(':
                if (scope_depth == 0)
                {
                    // Store current position and look for matching ')'.
                    size_t par_open = pos;
                    bool stop = false;
                    int par_depth = 1;

                    while (!stop && (pos = Fetch(sym)) != kTextEnd)
                    {
                        par_depth += (sym == '(' ? 1 : (sym == ')' ? -1 : 0));
                        stop = (sym == ')' && par_depth == 0 ? true : false);
                    }

                    if (stop && sym == ')')
                    {
                        // The function arglist may be followed by a "semantic" specifier, for example:
                        // float4 psMainD3D10( float4 screenSpace : SV_Position ) : COLOR {...}
                        if (Fetch(sym) != kTextEnd)
                        {
                            if (sym == '{' ||
                                (sym == ':' && !FetchToken().empty() && Fetch(sym) != kTextEnd && sym == '{'))
                            {
                                MoveCaret(par_open);
                                std::string name = RFetchToken();

                                // If the token preceding the found name is ':', it's a type specifier, not a function name.
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

        return (scope_depth == 0);
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
        while (caret_ < text_size_ && IsSpace(text_[caret_])) { caret_++; }
        sym = (caret_ < text_size_ ? text_[caret_] : 0);
        return (caret_ < text_size_ ? caret_++ : kTextEnd);
    }

    // Skips spaces/tabs/newlines. Shifts the caret position accordingly.
    // Returns updated caret position or TEXT_END if end of text is reached.
    size_t SkipSpaces()
    {
        while (caret_ < text_size_ && IsSpace(text_[caret_])) { caret_++; }
        return (caret_ < text_size_ ? caret_ : kTextEnd);
    }

    // Skips spaces/tabs/newlines in reverse direction. Shifts the caret position accordingly.
    // Returns updated caret position or TEXT_END if beginning of the text is reached.
    // The returned caret position points to the last space before a token (in reverse order).
    size_t RSkipSpaces()
    {
        while (caret_ > 0 && IsSpace(text_[--caret_])) {}
        return (caret_ + 1 > 0 ? ++caret_ : kTextEnd);
    }

    // Sets caret to the required position.
    void MoveCaret(size_t pos)
    {
        assert(pos < text_size_);
        caret_ = (pos < text_size_ ? pos : caret_);
    }

    // Fetches a token that consisting of letters and numbers, skipping the leading spaces.
    // If the symbol at current caret position is not a letter or number, returns this symbol.
    // If the end of text is reached, returns empty string.
    // Advances the caret position accordingly.
    std::string FetchToken()
    {
        std::string ret;
        if (SkipSpaces() != kTextEnd)
        {
            while (caret_ < text_size_ &&
                ((text_[caret_] >= 'A' && text_[caret_] <= 'z') ||
                (text_[caret_] >= '0' && text_[caret_] <= '9') || text_[caret_] == '_'))
            {
                ret.push_back(text_[caret_++]);
            }

            if (ret.empty() && caret_ < text_size_)
            {
                ret.push_back(text_[caret_++]);
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
        if (RSkipSpaces() != kTextEnd)
        {
            caret_--;
            while (caret_ + 1 > 0 &&
                ((text_[caret_] >= 'A' && text_[caret_] <= 'z') ||
                (text_[caret_] >= '0' && text_[caret_] <= '9') || text_[caret_] == '_'))
            {
                ret.push_back(text_[caret_--]);
            }

            if (ret.empty() && caret_ > 0)
            {
                ret.push_back(text_[caret_]);
            }
            else
            {
                caret_++;
            }
        }
        std::reverse(ret.begin(), ret.end());
        return ret;
    }

    std::string  text_;
    size_t       text_size_ = 0;
    size_t       caret_ = 0;
    const size_t kTextEnd = std::numeric_limits<size_t>::max();
};

// Callback for printing to stdout.
static void LoggingCallback(const std::string& s)
{
    RgLog::stdOut << s.c_str() << std::flush;
}

// Construct per-stage output file names for binary, ISA disassembly and statistics based on base file
// names specified by a user and target GPU name ("device"). "spvFileNames" contains the names of input
// spv files. The output ISA file names will be generated for corresponding non-empty spv file names.
static bool ConstructVkOutputFileNames(const Config& config,
    const std::string& base_bin_filename,
    const std::string& base_isa_filename,
    const std::string& base_stats_filename,
    const std::string& device,
    const BeVkPipelineFiles& spv_filenames,
    std::string& bin_filename,
    BeVkPipelineFiles& isa_filenames,
    BeVkPipelineFiles& stats_filenames)
{
    bool status = true;

    if (!base_bin_filename.empty())
    {
        status = KcUtils::ConstructOutFileName(base_bin_filename, "", (!config.should_avoid_binary_device_prefix ? device : ""),
            (config.should_avoid_binary_suffix ? "" : kStrVulkanBinaryFileExtension), bin_filename);
    }
    else if (KcUtils::IsNavi3Target(device) && !base_isa_filename.empty())
    {
        status = KcUtils::ConstructOutFileName(kStrDefaultFilenameOutputBinaryFileName,
                                               "",
                                               (!config.should_avoid_binary_device_prefix ? device : ""),
                                               (config.should_avoid_binary_suffix ? "" : kStrVulkanBinaryFileExtension),
                                               bin_filename);
    }

    for (int stage = 0; stage < BePipelineStage::kCount && status; stage++)
    {
        if (!spv_filenames[stage].empty() && status)
        {
            status = status && KcUtils::ConstructOutFileName(base_isa_filename, kVulkanStageFileSuffixDefault[stage], device,
                kStrVulkanIsaFileExtension, isa_filenames[stage]);

            status = status && KcUtils::ConstructOutFileName(base_stats_filename, kVulkanStageFileSuffixDefault[stage], device,
                kStrVulkanStatsFileExtension, stats_filenames[stage]);
        }
    }

    if (KcUtils::IsNavi3Target(device) && !isa_filenames.empty())
    {
        status = KcUtils::ConstructOutFileName(base_bin_filename,
                                               "",
                                               (!config.should_avoid_binary_device_prefix ? device : ""),
                                               (config.should_avoid_binary_suffix ? "" : kStrVulkanBinaryFileExtension),
                                               bin_filename);
    }

    assert(status);
    return status;
}

static void  LogPreStep(const std::string& msg, const std::string& device = "")
{
    RgLog::stdOut << msg << device << "... " << std::flush;
}

static void  LogResult(bool result)
{
    RgLog::stdOut << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl;
}

static void  LogErrorStatus(beStatus status, const std::string& errMsg)
{
    switch (status)
    {
    case kBeStatusSuccess:
        break;
    case kBeStatusVulkanGlslangLaunchFailed:
        RgLog::stdOut << kStrErrorVulkanFailedToLaunchGlslang << std::endl;
        break;
    case kBeStatusVulkanFrontendCompileFailed:
        RgLog::stdOut << kStrErrorVulkanFrontendCompilationFailed << std::endl;
        if (!errMsg.empty())
        {
            RgLog::stdOut << RgLog::noflush << kStrErrorVulkanGlslangErrorMessage << std::endl;
            RgLog::stdOut << errMsg << std::endl << RgLog::flush;
        }
        break;
    case kBeStatusVulkanBackendLaunchFailed:
        RgLog::stdOut << kStrErrorVulkanFailedToLaunchBackend << std::endl;
        break;
    case kBeStatusVulkanBackendCompileFailed:
        RgLog::stdOut << kStrErrorVulkanBackendFailure << std::endl;
        if (!errMsg.empty())
        {
            RgLog::stdOut << RgLog::noflush << kStrErrorVulkanBackendErrorMessage << std::endl;
            RgLog::stdOut << errMsg << std::endl << RgLog::flush;
        }
        break;
    case kBeStatusConstructIsaFileNameFailed:
        RgLog::stdOut << kStrErrorFailedToAdjustFileName << std::endl;
        break;
    case kBeStatusVulkanParseStatsFailed:
        RgLog::stdOut << kStrErrorVulkanFailedToConvertStats << std::endl;
        break;
    default:
        std::cout << std::endl << (errMsg.empty() ? kStrErrorUnknownCompilationStatus : errMsg) << std::endl;
        break;
    }
}

// Checks if specified file path is a SPIR-V binary file.
static bool IsSpvBinFile(const std::string& file_path)
{
    bool is_spv = false;

    if (KcUtils::FileNotEmpty(file_path))
    {
        std::ifstream file(file_path);
        if (file.good())
        {
            // Read the first 32-bit word of the file and check if it matches the SPIR-V binary magic number.
            uint32_t word;
            if (file.read(reinterpret_cast<char*>(&word), sizeof(word)) && file.good())
            {
                is_spv = (word == SPV_BINARY_MAGIC_NUMBER);
            }
        }
    }

    return is_spv;
}

// Extract the list of entry points and corresponding execution models from the SPIR-V binary file specified by "spvFilePath".
// Extracted entry points are returned in the "entries" vector.
// Returns "true" if succeeded or "false" otherwise.
static bool ExtractSpvEntries(const std::string& spv_file_path, spirv_cross::SmallVector<spirv_cross::EntryPoint>& entries)
{
    bool result = false;
    entries.clear();

    FILE *spv_file = std::fopen(spv_file_path.c_str(), "rb");
    if (spv_file != nullptr)
    {
        std::fseek(spv_file, 0L, SEEK_END);
        std::vector<uint32_t> spv(std::ftell(spv_file) / sizeof(uint32_t));
        std::rewind(spv_file);
        result = (std::fread(spv.data(), sizeof(uint32_t), spv.size(), spv_file) == spv.size());
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
//  "spvasm" --> SPIR-V text.
//  "vert", "frag", etc. --> GLSL.
//  other --> SPIR-V binary.
static RgVulkanInputType GetInputTypeForExt(std::string file_ext)
{
    RgVulkanInputType input_type = RgVulkanInputType::kUnknown;

    if (!file_ext.empty())
    {
        std::transform(file_ext.begin(), file_ext.end(), file_ext.begin(), [](const char& c) {return std::tolower(c); });
        if (std::find(kStrGlslFileExtensions.cbegin(), kStrGlslFileExtensions.cend(), file_ext) != kStrGlslFileExtensions.cend() ||
            std::find(kStrGlslFileExtensionsAdditional.cbegin(), kStrGlslFileExtensionsAdditional.cend(), file_ext) != kStrGlslFileExtensionsAdditional.cend())
        {
            input_type = RgVulkanInputType::kGlsl;
        }
        else if (file_ext == kStrVulkanHlslFileExtension)
        {
            input_type = RgVulkanInputType::kHlsl;
        }
        else if (file_ext == kStrVulkanSpirvFileExtension)
        {
            input_type = RgVulkanInputType::kSpirv;
        }
        else if (file_ext == kStrVulkanSpirvTextFileExtension)
        {
            input_type = RgVulkanInputType::kSpirvTxt;
        }
        else
        {
            input_type = RgVulkanInputType::kSpirv;
        }
    }

    return input_type;
}

// Detect the type of the given Vulkan input file by its extension.
static RgVulkanInputType DetectInputTypeByFileExt(const std::string& file_path)
{
    RgVulkanInputType input_type = RgVulkanInputType::kUnknown;

    // Get the file extension.
    size_t offset = file_path.rfind('.');
    const std::string& ext = (offset != std::string::npos && ++offset < file_path.size()) ? file_path.substr(offset) : "";

    // Try to detect the type of shader file by its extension.
    input_type = GetInputTypeForExt(ext);
    if (input_type == RgVulkanInputType::kUnknown)
    {
        RgLog::stdOut << kStrErrorVulkanFailedToDetectInputFileByExtension1 << std::endl;
    }

    return input_type;
}

// Detect the type of vulkan input files.
// Returns an array of input shader types corresponding to the pipeline stages.
static std::array<RgVulkanInputType, BePipelineStage::kCount>
DetectInputFiles(const Config& config)
{
    std::array<RgVulkanInputType, BePipelineStage::kCount> ret;
    ret.fill(RgVulkanInputType::kUnknown);

    // Identify the type of input files.
    // 1. If the input types for all stages is specified explicitly ("--hlsl", "--spv", "--spvtxt" options), assume that type.
    if (config.is_hlsl_input)
    {
        ret.fill(RgVulkanInputType::kHlsl);
    }
    else if (config.is_spv_input)
    {
        ret.fill(RgVulkanInputType::kSpirv);
    }
    else if (config.is_spv_txt_input)
    {
        ret.fill(RgVulkanInputType::kHlsl);
    }
    else
    {
        // 2. If the input type for all stages is not explicitly specified.
        std::array<RgVulkanInputType, BePipelineStage::kCount> user_file_types =
        { config.vert_shader_file_type, config.tesc_shader_file_type, config.tese_shader_file_type,
          config.geom_shader_file_type, config.frag_shader_file_type, config.comp_shader_file_type };

        BeVkPipelineFiles files = { config.vertex_shader, config.tess_control_shader, config.tess_evaluation_shader,
                                    config.geometry_shader, config.fragment_shader, config.compute_shader };

        // Check if per-stage file type is provided (for example, the "--vert-glsl" option is used).
        // If not, try to detect the shader file type by its extension:
        //  "hlsl" --> HLSL.
        //  "spvasm" --> SPIR-V text.
        //  "vert", "frag", etc. --> GLSL.
        //  other  --> SPIR-V binary.
        for (int stage = BePipelineStage::kVertex; stage < BePipelineStage::kCount; ++stage)
        {
            if (!files[stage].empty())
            {
                ret[stage] = (user_file_types[stage] != RgVulkanInputType::kUnknown ?
                    user_file_types[stage] : DetectInputTypeByFileExt(files[stage]));
            }
        }
    }

    return ret;
}

// Validates that if shader stage files are provided, they exist and are non-empty. Otherwise,
// logs an appropriate message to the console.
static bool IsShaderStageInputValid(const std::string& shader_input_file, const std::string& shader_stage_name)
{
    bool ret = false;
    if (KcUtils::FileNotEmpty(shader_input_file.c_str()))
    {
        ret = true;
    }
    else
    {
        // If shader stage files are provided, they must exist and be non-empty.
        static const std::string STR_VULKAN_ERROR_SOURCE_FILE_FOR = "Error: source file for ";
        static const std::string STR_VULKAN_ERROR_STAGE_NOT_EXIST_OR_EMPTY = " stage does not exist or is empty: ";

        std::cout << STR_VULKAN_ERROR_SOURCE_FILE_FOR
            << shader_stage_name
            << STR_VULKAN_ERROR_STAGE_NOT_EXIST_OR_EMPTY
            << shader_input_file
            << std::endl;
    }

    return ret;
};

// Determines if input files are required, and validates them if so.
static bool IsInputValid(const Config& config)
{
    bool ret = false;

    // Determine if an input file is required.
    bool is_input_file_required =
        (!config.isa_file.empty() ||
            !config.binary_output_file.empty() ||
            !config.analysis_file.empty() ||
            !config.livereg_analysis_file.empty() ||
            !config.block_cfg_file.empty() || !config.inst_cfg_file.empty());

    if (is_input_file_required)
    {
        // Note: order of possibleShaderStageFilePaths must be preserved to match
        // pipeline stages in the local std::array STR_PIPELINE_STAGE_FULL_NAMES.
        const std::vector<std::string> possible_shader_stage_file_paths =
        {
            config.vertex_shader,
            config.tess_control_shader,
            config.tess_evaluation_shader,
            config.geometry_shader,
            config.fragment_shader,
            config.compute_shader
        };

        // If input is required, user must provide at least one valid shader stage
        // We also need to make sure that no invalid stage combination was provided.
        bool any_shader_stage_provided = false;
        bool all_shader_stages_valid = true;
        for (uint8_t ii = 0; ii < BePipelineStage::kCount; ii++)
        {
            bool is_stage_provided = !possible_shader_stage_file_paths[ii].empty();
            if (is_stage_provided)
            {
                any_shader_stage_provided = true;
                all_shader_stages_valid &= IsShaderStageInputValid(possible_shader_stage_file_paths[ii], kStrPipelineStageNamesFull[ii]);
            }
        }

        if (!any_shader_stage_provided)
        {
            const std::string STR_VULKAN_ERROR_NO_INPUT_STAGES = "no valid input file provided for any pipeline stage.";
            std::cout << kStrErrorError << STR_VULKAN_ERROR_NO_INPUT_STAGES;
        }

        if (any_shader_stage_provided && all_shader_stages_valid)
        {
            // Pipeline state file is not provided.
            if (config.pso.empty())
            {
                std::cout << kStrWarningPso1 << kStrWarningPso2;
                if (config.compute_shader.empty())
                {
                    std::cout << kStrWarningPsoGraphics;
                }
                else
                {
                    std::cout << kStrWarningPsoCompute;
                }
                std::cout << kStrWarningPso3 << std::endl;
            }

            ret = true;
        }
    }
    else
    {
        // It is valid to provide no input if none is required.
        ret = true;
    }

    return ret;
}

void KcCliCommanderVulkan::RunCompileCommands(const Config& config, LoggingCallbackFunction)
{
    bool status = false;
    bool should_abort = false;
    std::string  spv_filename;

    should_abort = !IsInputValid(config);

    if (!config.pso_dx12.empty())
    {
        std::cout << kStrErrorVulkanGpsoOptionNotSupported << std::endl;
        should_abort = true;
    }

    if (!should_abort)
    {
        if (!config.icd_file.empty())
        {
            // If custom ICD location was used, notify the user.
            std::cout << kStrInfoVulkanUsingCustomIcdFile << config.icd_file << std::endl << std::endl;
        }

        if (!config.loader_debug.empty())
        {
            bool is_env_var_set = KcUtils::SetEnvrironmentVariable(kStrVulkanVkLoaderDebugEnvVarName, config.loader_debug);
            assert(is_env_var_set);
            if (!is_env_var_set)
            {
                // Notify the user.
                std::cout << kStrWarningVulkanFailedToSetEnvVar1 << kStrVulkanVkLoaderDebugEnvVarName << " " <<
                    kStrWarningVulkanFailedToSetEnvVar2 << std::endl;
            }
        }

        // Detect the action requested by user.
        if (!config.spv_bin.empty())
        {
            // Assembling SPIR-V text is required.
            status = AssembleSpv(config);
        }
        else if (!config.spv_txt.empty())
        {
            // Disassembling SPIR-V binary is requested.
            status = DisassembleSpv(config);
        }
        else if (!config.parsed_spv.empty())
        {
            // Parsing SPIR-V binary is requested.
            status = ParseSpv(config);
        }
        else
        {
            if ((status = InitRequestedAsicListVulkan(config)) == true)
            {
                // Detect the type of input files.
                auto input_file_types = DetectInputFiles(config);

                BeVkPipelineFiles inputFiles = { config.vertex_shader, config.tess_control_shader, config.tess_evaluation_shader,
                                                 config.geometry_shader, config.fragment_shader, config.compute_shader };

                BeVkPipelineFiles glslFiles, hlslFiles, spv_txt_files, spv_files;
                bool is_glsl_hlsl_files_found = false, found_spv_txt_files = false;

                // Collect shader files that have to be pre-compiled to SPIR-V binary format.
                for (int stage = BePipelineStage::kVertex; stage < BePipelineStage::kCount; ++stage)
                {
                    if (input_file_types[stage] == RgVulkanInputType::kGlsl)
                    {
                        glslFiles[stage] = inputFiles[stage];
                        is_glsl_hlsl_files_found = true;
                    }
                    else if (input_file_types[stage] == RgVulkanInputType::kHlsl)
                    {
                        hlslFiles[stage] = inputFiles[stage];
                        is_glsl_hlsl_files_found = true;
                    }
                    else if (input_file_types[stage] == RgVulkanInputType::kSpirvTxt)
                    {
                        spv_txt_files[stage] = inputFiles[stage];
                        found_spv_txt_files = true;
                    }
                    else
                    {
                        spv_files[stage] = inputFiles[stage];
                    }
                }

                // Pre-compile all glsl or hlsl input files to SPIR-V binaries.
                if (is_glsl_hlsl_files_found)
                {
                    status = status && CompileSourceToSpv(config, glslFiles, hlslFiles, spv_files);
                }

                // Pre-compile all SPIR-V text files to SPIR-V binaries.
                if (found_spv_txt_files)
                {
                    status = status && AssembleSpvTxtInputFiles(config, spv_txt_files, spv_files);
                }

                // Create a per-device copy of the build configuration.
                Config configPerDevice = config;

                if (asics_.empty())
                {
                    std::cout << kStrErrorVulkanNoTargetSpecified << std::endl;
                }
                else
                {
                    // Compile per-device.
                    for (const std::string& target : asics_)
                    {
                        // Compile for the specific target.
                        configPerDevice.asics.clear();
                        configPerDevice.asics.push_back(target);

                        // Now, compile the SPIR-V binaries.
                        if (status)
                        {
                            // Perform back-end compilation from a SPIR-V binary to ISA disassembly & statistics.
                            CompileSpvToIsaForDevice(configPerDevice, spv_files, target);
                        }
                    }

                     KcCLICommanderVulkanUtil util(output_metadata_, physical_adapter_name_, log_callback_, kVulkanStageFileSuffixDefault);
                     util.RunPostProcessingSteps(configPerDevice);
                }
            }
        }
    }
}

bool KcCliCommanderVulkan::RunPostCompileSteps(const Config& config)
{
    bool ret = false;

    KcCLICommanderVulkanUtil util(output_metadata_, physical_adapter_name_, log_callback_, kVulkanStageFileSuffixDefault);

    if (!config.session_metadata_file.empty())
    {
        ret = util.GenerateSessionMetadata(config);
        if (!ret)
        {
            RgLog::stdOut << kStrErrorFailedToGenerateSessionMetdata << std::endl;
        }
    }

    if (!config.should_retain_temp_files)
    {
        KcCLICommanderVulkanUtil::DeleteTempFiles(temp_files_);
    }

    return ret;
}

bool KcCliCommanderVulkan::PrintAsicList(const Config& config)
{
    std::set<std::string> all_gpus, matched_devices;

    bool result = GetSupportedTargets(config, matched_devices, config.print_process_cmd_line);
    assert(result);

    result = result && KcUtils::PrintAsicList(matched_devices);

    return result;
}

bool KcCliCommanderVulkan::ListEntries(const Config& config, LoggingCallbackFunction callback)
{
    std::string prepro_text, err_msg;
    bool ret = false;

    if (config.input_files.size() != 1)
    {
        RgLog::stdOut << kStrErrorSingleInputFileExpected << std::endl;
    }
    else
    {
        const std::string& file_path = config.input_files[0];
        bool isHlsl = (config.is_hlsl_input || DetectInputTypeByFileExt(file_path) == RgVulkanInputType::kHlsl);
        if (isHlsl)
        {
            // Pre-process the input file.
            beStatus status = beProgramBuilderVulkan::PreprocessSource(config, config.compiler_bin_path, config.input_files[0], true,
                config.print_process_cmd_line, prepro_text, err_msg);
            if (status != kBeStatusSuccess || prepro_text.empty())
            {
                RgLog::stdOut << kStrErrorVulkanPrepprocessFailed << config.input_files[0] << std::endl;
                if (!err_msg.empty())
                {
                    RgLog::stdOut << kStrErrorVulkanGlslangErrorMessage << std::endl << err_msg << std::endl;
                }
            }
            else
            {
                // Extract the function names from the preprocessed text and dump them to stdout.
                std::vector<std::string> func_names;
                kcHlslParser parser(prepro_text);
                if (parser.GetFuncNames(func_names))
                {
                    for (const std::string& name : func_names)
                    {
                        RgLog::stdOut << name << std::endl;
                    }
                    ret = true;
                }
                else
                {
                    RgLog::stdOut << kStrErrorVulkanFailedToExtractHlslEntries << std::endl;
                }
            }
        }
        else
        {
            RgLog::stdOut << kStrErrorVulkanEntryDetectionWrongLanguage << std::endl;
        }
    }

    return ret;
}

bool KcCliCommanderVulkan::GetSupportedTargets(const Config& config, std::set<std::string>& targets, bool print_cmd /*= false*/)
{
    bool result = false;
    std::string error_msg;
    std::set<std::string> vulkan_devices;
    targets.clear();
    beStatus status = beProgramBuilderVulkan::GetVulkanDriverTargetGPUs(config.loader_debug, config.icd_file, vulkan_devices, print_cmd, error_msg);
    result = (status == kBeStatusSuccess);

    if (result && !vulkan_devices.empty())
    {
        std::vector<GDT_GfxCardInfo> card_list;
        std::set<std::string> known_arch_names;

        // Get the list of known GPU architectures from DeviceInfo.
        if ((result = BeUtils::GetAllGraphicsCards(card_list, known_arch_names, true)) == true)
        {
            // Filter the Vulkan devices: keep only those devices that are present in the DeviceInfo.
            for (auto it = vulkan_devices.begin(); it != vulkan_devices.end(); )
            {
                // Some device names are returned by Vulkan driver in non-standard form.
                // Try looking for corrected name in the map. If found, replace the device name
                // in the "vulkanDevices" set with the corrected name.
                auto corrected_name = std::find_if(kPalDeviceNameMapping.cbegin(), kPalDeviceNameMapping.cend(),
                    [&](const std::pair<std::string, std::string>& device) { return (device.first == *it); });

                const std::string& device = (corrected_name == kPalDeviceNameMapping.end() ? *it : corrected_name->second);
                if (corrected_name != kPalDeviceNameMapping.end())
                {
                    vulkan_devices.insert(corrected_name->second);
                }

                it = (known_arch_names.find(device) == known_arch_names.end() || corrected_name != kPalDeviceNameMapping.end() ?
                    vulkan_devices.erase(it) : ++it);
            }
        }
    }
    else
    {
        RgLog::stdOut << kStrErrorVulkanFailedToGetTargets << std::endl;
        if (!error_msg.empty())
        {
            RgLog::stdOut << kStrErrorVulkanBackendErrorMessage << std::endl << error_msg << std::endl;
        }
    }

    targets = vulkan_devices;
    result = !targets.empty();

    return result;
}

bool KcCliCommanderVulkan::GenerateVulkanVersionInfo(const Config& config, const std::string& filename, bool print_cmd /*= false*/)
{
    std::set<std::string> targets;

    // Get the list of supported GPUs for current mode.
    bool result = GetSupportedTargets(config, targets, print_cmd);
    assert(result);

    // Add the list of supported GPUs to the Version Info file.
    result = result && KcXmlWriter::AddVersionInfoGPUList(RgaMode::kModeVulkan, targets, filename);
    assert(result);

    return result;
}

bool KcCliCommanderVulkan::GenerateSystemVersionInfo(const Config& config, const std::string& filename, bool print_cmd)
{
    bool result = false;
    std::vector<BeVkPhysAdapterInfo> adapter_info;
    std::string error_msg;

    beStatus status = beProgramBuilderVulkan::GetPhysicalGPUs(config.icd_file, adapter_info, print_cmd, error_msg);
    if (status == kBeStatusSuccess)
    {
        result = KcXmlWriter::AddVersionInfoSystemData(adapter_info, filename);
        assert(result);
    }

    return result;
}

bool KcCliCommanderVulkan::InitRequestedAsicListVulkan(const Config& config)
{
    bool result = false;
    bool use_physical_adapter = (config.asics.empty());

    std::set<std::string> vulkan_devices, matched_targets;

    // Get the list of supported target devices.
    result = GetSupportedTargets(config, vulkan_devices, config.print_process_cmd_line);
    if (!result)
    {
        RgLog::stdOut << kStrErrorCannotExtractSupportedDeviceList << std::endl;
    }
    else
    {
        if (use_physical_adapter)
        {
            std::string error_msg;
            std::vector<BeVkPhysAdapterInfo> physical_adapters;
            beStatus status = beProgramBuilderVulkan::GetPhysicalGPUs(config.icd_file, physical_adapters,
                config.print_process_cmd_line, error_msg);
            if (status == kBeStatusSuccess)
            {
                physical_adapter_name_ = physical_adapters[0].name;
            }
            else
            {
                RgLog::stdOut << kStrErrorVulkanFailedToGetAdapters << std::endl;
                result = false;
            }
        }
        else
        {
            if ((result = InitRequestedAsicList(config.asics, config.mode, vulkan_devices, matched_targets, false)) == true)
            {
                asics_ = matched_targets;
            }
        }
    }

    if (asics_.empty() && !use_physical_adapter)
    {
        RgLog::stdOut << kStrErrorVulkanDevicesNotSupported <<
            [&]() {std::string s; for (auto& d : config.asics) s += (d + " "); return s; }() << std::endl;
    }

    return result;
}

bool KcCliCommanderVulkan::CompileSourceToSpv(const Config& conf, const BeVkPipelineFiles& glsl_files,
    const BeVkPipelineFiles& hlsl_files, BeVkPipelineFiles& out_spv_files)
{
    bool result = true;
    std::string error_msg;
    BeVkPipelineFiles src_files;

    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
    {
        bool is_hlsl = !hlsl_files[stage].empty();
        src_files[stage] = (is_hlsl ? hlsl_files[stage] : glsl_files[stage]);
        if (!src_files[stage].empty())
        {
            std::string msg = kStrInfoVulkanPrecompiling1 + kStrPipelineStageNamesFull[stage] + kStrInfoVulkanPrecompiling2;
            msg += ("(" + src_files[stage] + ")");
            msg += kStrInfoVulkanPrecompiling3;

            // Construct a name for temporary spv file.
            beStatus status = kBeStatusSuccess;
            if (KcUtils::ConstructOutFileName("", kVulkanStageFileSuffixDefault[stage], kStrVulkanFileSuffixAllDevices,
                kStrVulkanSpirvFileExtension, out_spv_files[stage]))
            {
                temp_files_.push_back(out_spv_files[stage]);
            }
            else
            {
                status = kBeStatusVulkanConstructOutFileNameFailed;
            }

            // Notify the user about the front-end compilation.
            msg += (" (" + out_spv_files[stage] + ")");
            LogPreStep(msg);

            if (status == kBeStatusSuccess)
            {
                BePipelineStage pipeline_stage = static_cast<BePipelineStage>(stage);
                status = beProgramBuilderVulkan::CompileSrcToSpirvBinary(conf, src_files[stage],
                    out_spv_files[stage], pipeline_stage, is_hlsl, error_msg);
            }

            // Check if output spv file has not been generated for some reason.
            if (status == kBeStatusSuccess && !KcUtils::FileNotEmpty(out_spv_files[stage]))
            {
                status = kBeStatusVulkanFrontendCompileFailed;
            }

            assert(status == kBeStatusSuccess);
            LogResult(status == kBeStatusSuccess);

            if (status != kBeStatusSuccess)
            {
                LogErrorStatus(status, error_msg);
                result = false;
                break;
            }
        }
    }

    // Store the input file names to the output metadata.
    if (result)
    {
        StoreInputFilesToOutputMD(src_files);
    }

    return result;
}

void KcCliCommanderVulkan::CompileSpvToIsaForDevice(const Config& config, const BeVkPipelineFiles& spv_files,
    const std::string& device, bool is_physical_adapter)
{
    const std::string& device_suffix = (is_physical_adapter ? "" : device);
    LogPreStep(kStrInfoCompiling, device);
    BeVkPipelineFiles  isa_files, stats_files;

    bool result = false;
    std::string isa_file_base_name = config.isa_file;
    std::string stats_file_base_name = config.analysis_file;
    std::string bin_file_base_name = config.binary_output_file;
    std::string bin_file_name, validation_filename, error_msg;
    beStatus status = kBeStatusGeneralFailed;

    // Construct names for output files.
    result = ConstructVkOutputFileNames(config, bin_file_base_name, isa_file_base_name,
        stats_file_base_name, device_suffix, spv_files, bin_file_name, isa_files, stats_files);

    if (result && !config.vulkan_validation.empty())
    {
        // Construct name for Vulkan validation info output file.
        validation_filename = KcUtils::ConstructTempFileName(kStrVulkanTempValidationInfoFilename,
            kStrVulkanValidationInfoFileExtension);
        result = !validation_filename.empty();
    }

    // A flag that indicates if we fell back to vk-spv-offline mode.
    bool is_vk_offline = false;

    if (result)
    {
        // If the Vulkan driver uses non-standard name for this device, convert it back to the driver format.
        auto corrected_device = std::find_if(kPalDeviceNameMapping.cbegin(), kPalDeviceNameMapping.cend(),
            [&](const std::pair<std::string, std::string>& d) { return (d.second == device); });

        const std::string& vulkan_device = (corrected_device == kPalDeviceNameMapping.end() ? device : corrected_device->first);

        // Remove ISA output files if they exist before attempting to compile.
        for (const std::string& out_filename : isa_files)
        {
            if (!out_filename.empty())
            {
                KcUtils::DeleteFile(out_filename);
            }
        }

        // Remove stats files if they exist before attempting to compile.
        for (const std::string& out_filename : stats_files)
        {
            if (!out_filename.empty())
            {
                KcUtils::DeleteFile(out_filename);
            }
        }

        // Perform the compilation.
        status = beProgramBuilderVulkan::CompileSpirv(config.loader_debug, spv_files, isa_files, stats_files, bin_file_name, config.pso,
            config.icd_file, validation_filename, config.vulkan_validation,
            (is_physical_adapter ? "" : vulkan_device), config.print_process_cmd_line, error_msg);

        if (status != kBeStatusSuccess)
        {
            // Report the backend error.
            std::cout << std::endl << error_msg << std::endl;
            error_msg.clear();

            // Warn the user.
            std::cout << kStrWarningVulkanFallbackToVkOfflineMode << std::endl << std::endl;
            std::cout << kStrInfoVulkanFallingBackToOfflineMode << std::endl;
            is_vk_offline = true;

            // Adjust the config file to the vk-spv-offline mode, and target only the current device.
            Config vk_offline_config = config;
            vk_offline_config.mode = RgaMode::kModeVkOfflineSpv;
            vk_offline_config.input_files.clear();
            vk_offline_config.asics.clear();
            vk_offline_config.asics.push_back(device);

            // Adjust the configuration structure to reference the compiled SPIR-V binaries.
            vk_offline_config.vertex_shader = spv_files[BePipelineStage::kVertex];
            vk_offline_config.tess_control_shader = spv_files[BePipelineStage::kTessellationControl];
            vk_offline_config.tess_evaluation_shader = spv_files[BePipelineStage::kTessellationEvaluation];
            vk_offline_config.geometry_shader = spv_files[BePipelineStage::kGeometry];
            vk_offline_config.fragment_shader = spv_files[BePipelineStage::kFragment];
            vk_offline_config.compute_shader = spv_files[BePipelineStage::kCompute];

            // Fallback to using vk-spv-offline.
            KcCLICommanderVkOffline commander;
            commander.RunCompileCommands(vk_offline_config, LoggingCallback);

            // Verify that the compilation succeeded.
            bool is_build_success = true;
            for (int stage = 0; stage < BePipelineStage::kCount; stage++)
            {
                if (!spv_files[stage].empty())
                {
                    if ((!vk_offline_config.isa_file.empty() && !KcUtils::FileNotEmpty(isa_files[stage])) ||
                        (!vk_offline_config.analysis_file.empty() && !KcUtils::FileNotEmpty(stats_files[stage])))
                    {
                        is_build_success = false;
                        break;
                    }
                }
            }

            // If the compilation succeeded - mark this as a success.
            if (is_build_success)
            {
                status = kBeStatusSuccess;
            }
        }

        if (!is_vk_offline && status == kBeStatusSuccess)
        {
            // Notify the user about shader merge if happened.
            if (KcUtils::IsNaviTarget(device) || KcUtils::IsVegaTarget(device))
            {
                bool is_first_msg = true;
                if (!spv_files[BePipelineStage::kGeometry].empty() && spv_files[BePipelineStage::kTessellationEvaluation].empty())
                {
                    if (BeUtils::IsFilesIdentical(isa_files[BePipelineStage::kVertex], isa_files[BePipelineStage::kGeometry]))
                    {
                        if (is_first_msg)
                        {
                            std::cout << std::endl;
                            is_first_msg = false;
                        }
                        std::cout << kStrInfoVulkanMergedShadersGeometryVertex << std::endl;
                     }
                    is_first_msg = true;
                }
                else if (!spv_files[BePipelineStage::kGeometry].empty() && !spv_files[BePipelineStage::kTessellationEvaluation].empty())
                {
                    if (BeUtils::IsFilesIdentical(isa_files[BePipelineStage::kTessellationEvaluation], isa_files[BePipelineStage::kGeometry]))
                    {
                        if (is_first_msg)
                        {
                            std::cout << std::endl;
                            is_first_msg = false;
                        }
                        std::cout << kStrInfoVulkanMergedShadersGeometryTessellationEvaluation << std::endl;
                    }
                }

                if (!spv_files[BePipelineStage::kTessellationControl].empty())
                {
                    if (BeUtils::IsFilesIdentical(isa_files[BePipelineStage::kVertex], isa_files[BePipelineStage::kTessellationControl]))
                    {
                        if (is_first_msg)
                        {
                            std::cout << std::endl;
                            is_first_msg = false;
                        }
                        std::cout << kStrInfoVulkanMergedShadersTessellationControlVertex << std::endl;
                    }
                }
            }

            status = KcCLICommanderVulkanUtil::ConvertStats(isa_files, stats_files, config, device);
        }

        if (status == kBeStatusSuccess)
        {
            StoreOutputFilesToOutputMD(device, spv_files, isa_files, stats_files);
        }

        // If temporary ISA files are used, add their paths to the list of temporary files.
        if (!is_vk_offline && isa_file_base_name.empty())
        {
            std::copy_if(isa_files.cbegin(), isa_files.cend(), std::back_inserter(temp_files_),
                [&](const std::string& s) { return !s.empty(); });

            if (KcUtils::IsNavi3Target(device))
            {
                temp_files_.push_back(bin_file_name);
            }            
        }
    }
    else
    {
        status = kBeStatusConstructIsaFileNameFailed;
    }

    // Only print the status messages if we did not fall back
    // to vk-spv-offline mode, since otherwise logging happens in that mode.
    if (!is_vk_offline)
    {
        LogResult(status == kBeStatusSuccess);
        LogErrorStatus(status, error_msg);
    }
}

bool KcCliCommanderVulkan::AssembleSpv(const Config& conf)
{
    std::string input_spv_txt_file, error_msg;
    if (conf.input_files.size() == 1)
    {
        input_spv_txt_file = conf.input_files[0];
    }
    else if (conf.input_files.empty())
    {
        // User did not provide output path.
        RgLog::stdOut << kStrErrorVulkanAssembleNoOutputFile << std::endl;
    }
    else
    {
        // User provided too many arguments.
        RgLog::stdOut << kStrErrorVulkanInvalidNumArgs << std::endl;
    }

    bool result = !input_spv_txt_file.empty() && !conf.spv_bin.empty();
    if (result)
    {
        LogPreStep(kStrInfoVulkanAssemblingSpirv + input_spv_txt_file);

        beStatus status = beProgramBuilderVulkan::AssembleSpv(conf.compiler_bin_path, input_spv_txt_file, conf.spv_bin,
            conf.print_process_cmd_line, error_msg);

        result = (status == kBeStatusSuccess);
        LogResult(result);

        if (status != kBeStatusSuccess)
        {
            RgLog::stdOut << kStrErrorVulkanFailedToAssembleSpirv << input_spv_txt_file << std::endl;

            if (!error_msg.empty())
            {
                RgLog::stdOut << kStrErrorVulkanSpirvAssemblerErrorMessage << std::endl << error_msg << std::endl;
            }
        }
    }

    return result;
}

bool KcCliCommanderVulkan::DisassembleSpv(const Config& conf)
{
    std::string error_msg;
    std::string input_spv_file, output_spv_dis_file;

    // If input file is empty, consider the argument of "--disassemble-spv" option as an input file and dump
    // disassembly text to the stdout.
    if (conf.input_files.empty())
    {
        input_spv_file = conf.spv_txt;
    }
    else
    {
        input_spv_file = conf.input_files[0];
        output_spv_dis_file = conf.spv_txt;
    }

    bool result = !input_spv_file.empty();
    if (result)
    {
        LogPreStep(kStrInfoVulkanDisassemblingSpirv + input_spv_file);
        beStatus status = beProgramBuilderVulkan::DisassembleSpv(conf.compiler_bin_path, input_spv_file, output_spv_dis_file,
            conf.print_process_cmd_line, error_msg);

        result = (status == kBeStatusSuccess);
        LogResult(result);

        if (status != kBeStatusSuccess)
        {
            RgLog::stdOut << kStrErrorVulkanFailedToDisassembleSpirv << input_spv_file << std::endl;

            if (!error_msg.empty())
            {
                RgLog::stdOut << kStrErrorVulkanSpirvDisassemblerErrorMessage << std::endl << error_msg << std::endl;
            }
        }
    }

    return (result);
}

// Dump the SPIR-V info to the specified output stream (file or stdout).
static bool DumpSpvInfo(const std::multimap<spv::ExecutionModel, std::string>& entries, std::ostream& out_stream, const std::string& title)
{
    bool result = true;

    RgLog::stdOut << title << std::endl << std::endl;

    for (auto exec_model = entries.cbegin(); exec_model != entries.cend(); exec_model = entries.upper_bound(exec_model->first))
    {
        auto stage = PIPELINE_STAGE_FOR_SPV_EXEC_MODEL.find(exec_model->first);
        assert(stage != PIPELINE_STAGE_FOR_SPV_EXEC_MODEL.cend());
        if (stage != PIPELINE_STAGE_FOR_SPV_EXEC_MODEL.cend())
        {
            out_stream << kStrPipelineStageNames[stage->second] << ":";
            auto stage_entries = entries.equal_range(exec_model->first);
            for (auto entry = stage_entries.first; entry != stage_entries.second; entry++)
            {
                out_stream << " \"" << entry->second << "\"";
            }
            out_stream << std::endl;
        }
        else
        {
            RgLog::stdOut << kStrErrorVulkanNoPipelineStageForSpirvExecModel << std::endl;
            result = false;
            break;
        }
    }

    return result;
}

bool KcCliCommanderVulkan::ParseSpv(const Config& conf)
{
    spirv_cross::SmallVector<spirv_cross::EntryPoint> entries;
    std::string input_spv_file, output_info_file;
    bool result = false;

    // If input file is empty, consider the argument of "--parse-spv" option as an input file and dump
    // disassembly text to the stdout.
    if (conf.input_files.empty())
    {
        input_spv_file = conf.parsed_spv;
        result = true;
    }
    else if (conf.input_files.size() == 1)
    {
        input_spv_file = conf.input_files[0];
        output_info_file = conf.parsed_spv;
        result = true;
    }
    else
    {
        RgLog::stdOut << kStrErrorSingleInputFileExpected << std::endl;
    }

    if (result)
    {
        if ((result = IsSpvBinFile(input_spv_file)) == true)
        {
            RgLog::stdOut << kStrInfoVulkanParsingSpirv << input_spv_file << "... ";
            result = ExtractSpvEntries(input_spv_file, entries);
            RgLog::stdOut << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl << std::endl;
        }
        else
        {
            RgLog::stdOut << kStrErrorVulkanFileIsNotSpirvBinary << input_spv_file << std::endl;
        }
    }

    if (result)
    {
        std::multimap<spv::ExecutionModel, std::string> entry_map;

        // Classify entries by shader stage (execution model).
        for (const spirv_cross::EntryPoint& entry : entries)
        {
            entry_map.insert({ entry.execution_model, entry.name });
        }

        if (output_info_file.empty())
        {
            result = DumpSpvInfo(entry_map, std::cout, kStrVulkanSpirvInfo);
        }
        else
        {
            std::ofstream out_file(output_info_file);
            if (out_file.good())
            {
                const std::string title = kStrVulkanSpirvInfoSavedToFile + output_info_file;
                result = DumpSpvInfo(entry_map, out_file, title);
            }
            else
            {
                std::cerr << kStrErrorCannotOpenFileForWriteA << output_info_file << kStrErrorCannotOpenFileForWriteB << std::endl;
                result = false;
            }
        }
    }

    return result;
}

bool KcCliCommanderVulkan::AssembleSpvTxtInputFiles(const Config& config, const BeVkPipelineFiles& spv_txt_files, BeVkPipelineFiles& out_spv_files)
{
    bool result = true;
    std::string error_msg;

    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
    {
        if (!spv_txt_files[stage].empty())
        {
            LogPreStep(kStrInfoVulkanPrecompiling1 + kStrPipelineStageNames[stage] + kStrInfoVulkanPrecompiling2);

            // Construct a name for temporary spv file.
            result = KcUtils::ConstructOutFileName("", kVulkanStageFileSuffixDefault[stage], "", kStrVulkanSpirvFileExtension, out_spv_files[stage]);

            if (result)
            {
                beStatus status = beProgramBuilderVulkan::AssembleSpv(config.compiler_bin_path, spv_txt_files[stage],
                    out_spv_files[stage], config.print_process_cmd_line, error_msg);
                LogResult(status == kBeStatusSuccess);

                if (status != kBeStatusSuccess)
                {
                    result = false;
                    RgLog::stdOut << kStrErrorVulkanFailedToAssembleSpirv << spv_txt_files[stage] << std::endl;

                    if (!error_msg.empty())
                    {
                        RgLog::stdOut << kStrErrorVulkanSpirvAssemblerErrorMessage << std::endl << error_msg << std::endl;
                        error_msg.clear();
                    }
                }

                temp_files_.push_back(out_spv_files[stage]);
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
        StoreInputFilesToOutputMD(spv_txt_files);
    }

    return result;
}

bool KcCliCommanderVulkan::GenerateSessionMetadata(const Config& config) const
{
    return KcXmlWriter::GenerateVulkanSessionMetadataFile(config.session_metadata_file, output_metadata_);
}

bool KcCliCommanderVulkan::ParseIsaFilesToCSV(bool line_numbers, const std::string& device_string, RgVkOutputMetadata& metadata)
{
    bool ret = true;

    // Step through existing output items to determine which files to generate CSV ISA for.
    for (RgOutputFiles& output_file : metadata)
    {
        if (!output_file.input_file.empty())
        {
            std::string isa, parsed_isa, parsed_isa_filename;
            bool        status = KcUtils::ReadTextFile(output_file.isa_file, isa, nullptr);

            if (status)
            {
                // Convert the ISA text to CSV format.
                if ((status = GetParsedIsaCsvText(isa, device_string, line_numbers, parsed_isa)) == true)
                {
                    status = (KcUtils::GetParsedISAFileName(output_file.isa_file, parsed_isa_filename) == beKA::kBeStatusSuccess);
                    if (status)
                    {
                        // Attempt to write the ISA CSV to disk.
                        status = (WriteIsaToFile(parsed_isa_filename, parsed_isa) == beKA::kBeStatusSuccess);
                        if (status)
                        {
                            // Update the session metadata output to include the path to the ISA CSV.
                            output_file.isa_csv_file = parsed_isa_filename;
                        }
                    }
                }

                if (!status)
                {
                    RgLog::stdOut << kStrErrorFailedToConvertToCsvFormat << output_file.isa_file << std::endl;
                }
            }
            ret &= status;
        }
    }

    return ret;
}

bool KcCliCommanderVulkan::PerformLiveRegAnalysis(const Config& conf, const std::string& device, RgVkOutputMetadata& device_md)
{
    bool  ret = true;

    const std::string& device_suffix = (conf.asics.empty() && !physical_adapter_name_.empty() ? "" : device);
    gtString           device_gtstr;
    device_gtstr << device.c_str();

    std::cout << kStrInfoPerformingLiveregAnalysis1 << device << "... " << std::endl;

    for (int stage = 0; stage < BePipelineStage::kCount && ret; stage++)
    {
        RgOutputFiles& stage_md = device_md[stage];
        if (!stage_md.input_file.empty())
        {
            std::string out_file_name;
            gtString    out_filename_gtstr, isa_filename_gtstr;

            // Construct a name for the livereg output file.
            ret = KcUtils::ConstructOutFileName(conf.livereg_analysis_file,
                                                kVulkanStageFileSuffixDefault[stage],
                                                device_suffix,
                                                kStrDefaultExtensionLivereg,
                                                out_file_name,
                                                !KcUtils::IsDirectory(conf.livereg_analysis_file));

            if (ret && !out_file_name.empty())
            {
                out_filename_gtstr << out_file_name.c_str();
                isa_filename_gtstr << stage_md.isa_file.c_str();

                KcUtils::PerformLiveRegisterAnalysis(isa_filename_gtstr, device_gtstr, out_filename_gtstr, log_callback_, conf.print_process_cmd_line);
                ret                   = BeUtils::IsFilePresent(out_file_name);
                stage_md.livereg_file = out_file_name;
            }
            else
            {
                RgLog::stdOut << kStrErrorFailedCreateOutputFilename << std::endl;
            }
        }
    }

    LogResult(ret);

    return ret;
}

bool KcCliCommanderVulkan::ExtractCFG(const Config& config, const std::string& device, const RgVkOutputMetadata& device_md) const
{
    bool ret = true;

    const std::string& device_suffix = (config.asics.empty() && !physical_adapter_name_.empty() ? "" : device);
    bool               per_inst_cfg  = (!config.inst_cfg_file.empty());
    gtString           device_gtstr;
    device_gtstr << device.c_str();

    std::cout << (per_inst_cfg ? kStrInfoContructingPerInstructionCfg1 : kStrInfoContructingPerBlockCfg1) << device << "..." << std::endl;

    for (int stage = 0; stage < BePipelineStage::kCount && ret; stage++)
    {
        const RgOutputFiles& stage_md = device_md[stage];
        if (!stage_md.input_file.empty())
        {
            std::string out_filename;
            gtString    out_filename_gtstr, isa_filename_gtstr;

            // Construct a name for the CFG output file.
            const std::string cfg_output_file = (per_inst_cfg ? config.inst_cfg_file : config.block_cfg_file);
            ret                               = KcUtils::ConstructOutFileName(cfg_output_file,
                                                kVulkanStageFileSuffixDefault[stage],
                                                device_suffix,
                                                kStrDefaultExtensionDot,
                                                out_filename,
                                                !KcUtils::IsDirectory(cfg_output_file));

            if (ret && !out_filename.empty())
            {
                out_filename_gtstr << out_filename.c_str();
                isa_filename_gtstr << stage_md.isa_file.c_str();

                KcUtils::GenerateControlFlowGraph(
                    isa_filename_gtstr, device_gtstr, out_filename_gtstr, log_callback_, per_inst_cfg, config.print_process_cmd_line);
                ret = BeUtils::IsFilePresent(out_filename);
            }
            else
            {
                RgLog::stdOut << kStrErrorFailedCreateOutputFilename << std::endl;
            }
        }
    }

    LogResult(ret);

    return ret;
}

void KcCliCommanderVulkan::StoreInputFilesToOutputMD(const BeVkPipelineFiles& input_files)
{
    for (const std::string& device : asics_)
    {
        // If the output metadata for "device" does not exist, create a new node.
        // Otherwise, just add new input files to the existing node.
        if (output_metadata_.find(device) == output_metadata_.end())
        {
            RgVkOutputMetadata out_md;
            for (int stage = 0; stage < BePipelineStage::kCount; stage++)
            {
                RgOutputFiles out_files;
                out_files.device = device;
                out_files.entry_type = kVulkanOglStageEntryTypes[stage];
                out_files.input_file = input_files[stage];
                out_md[stage] = out_files;
            }

            output_metadata_[device] = out_md;
        }
        else
        {
            for (int stage = 0; stage < BePipelineStage::kCount; stage++)
            {
                RgOutputFiles& out_files = output_metadata_[device][stage];
                if (!input_files[stage].empty())
                {
                    assert(out_files.input_file.empty());
                    out_files.input_file = input_files[stage];
                }
            }
        }
    }
}

void KcCliCommanderVulkan::StoreOutputFilesToOutputMD(const std::string& device, const BeVkPipelineFiles& spvFiles,
    const BeVkPipelineFiles& isaFiles, const BeVkPipelineFiles& statsFiles)
{
    // Check if the output Metadata for this device already exists.
    // It exists if some of shader files are GLSL/HLSL or SPIR-V text files. In that case,
    // the metadata has been created during pre-compiling the shaders to SPIR-V binary format
    bool device_md_exists = (output_metadata_.find(device) != output_metadata_.end());
    if (!device_md_exists)
    {
        RgVkOutputMetadata md;
        RgOutputFiles out_files(device);
        md.fill(out_files);
        output_metadata_[device] = md;
    }

    RgVkOutputMetadata& device_md = output_metadata_[device];
    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
    {
        if (!spvFiles[stage].empty())
        {
            // If the "input file" in the metadata for this stage is non-empty, keep it there (it's the path to the
            // GLSL/HLSL/spv-text file that was pre-compiled to a temporary SPIR-V binary file).
            // If it's empty, store the spv file path as an "input file".
            RgOutputFiles& stage_md = device_md[stage];
            if (stage_md.input_file.empty())
            {
                stage_md.input_file = spvFiles[stage];
                stage_md.entry_type = kVulkanOglStageEntryTypes[stage];
                stage_md.device = device;
            }
            stage_md.isa_file = isaFiles[stage];
            stage_md.stats_file = statsFiles[stage];
        }
    }
}
