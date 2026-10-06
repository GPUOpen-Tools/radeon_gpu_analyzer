//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for parsing binary code object disassembly.
//=============================================================================

// C++.
#include <algorithm>
#include <cassert>
#include <cctype>
#include <unordered_set>

// Infrastructure.
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_lightning.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary_parser.h"
#include "radeon_gpu_analyzer_cli/kc_utils_lightning.h"

static const char* kStrErrorCannotParseDisassembly          = "Error: failed to parse LLVM disassembly.";
static const char* kStrErrorNoShaderFoundInDisassembly      = "Error: LLVM disassembly does not contatin valid kernels.";
static const char* kAmdgpuDisDotTextToken                   = ".text";
static const char* kAmdgpuDisShaderEndToken                 = "_symend:";
static const char* kAmdgpuDisDotSizeToken                   = ".size";
static const char* kAmdgpuDisKernelNameStartToken           = "_symend-";
static const char* kAmdgpuDisKernelNameStartTokenWithQuotes = "_symend\"-";
static const char* kAmdgpuDisShaderEndTokenNoColon          = "_symend";

beKA::beStatus KcCliAmdgpuDisComputeStrategy::Disassemble(const std::string& bin_file,
                                                          const std::vector<std::string>&,
                                                          const std::vector<SubstituteSourcePath>&,
                                                          bool         verbose,
                                                          std::string& text_disassembly)
{
    KcUtilsBinary::LogPreStep(kStrInfoDisassemblingBinary, bin_file);

    std::string    error_txt;
    beKA::beStatus ret = KcUtils::InvokeAmdgpudis(bin_file, verbose, text_disassembly, error_txt) ? beKA::beStatus::kBeStatusSuccess
                                                                                                  : beKA::beStatus::kBeStatusVulkanAmdgpudisLaunchFailed;
    if (text_disassembly.empty())
    {
        ret = beKA::beStatus::kBeStatusVulkanAmdgpudisLaunchFailed;
    }

    KcUtilsBinary::LogResult(ret == beKA::beStatus::kBeStatusSuccess);
    KcUtilsBinary::LogErrorStatus(ret, error_txt);
    return ret;
}

beKA::beStatus KcCliAmdgpuDisGraphicsStrategy::ParseKernels(const std::string&                  text_disassembly,
                                                            std::map<std::string, std::string>& kernel_to_disassembly,
                                                            std::string&                        error_msg) const
{
    beKA::beStatus status = beKA::beStatus::kBeStatusGeneralFailed;
    status                = beProgramBuilderVulkan::ParseAmdgpudisOutput(text_disassembly, kernel_to_disassembly, error_msg);
    if (status != beKA::beStatus::kBeStatusSuccess)
    {
        std::map<std::string, std::string> dxr_kernel_to_disassembly;
        std::string                        dxr_error_str;
        if (KcCliAmdgpuDisComputeStrategy{}.ParseKernels(text_disassembly, dxr_kernel_to_disassembly, dxr_error_str) == beKA::beStatus::kBeStatusSuccess)
        {
            status                = beKA::beStatus::kBeStatusSuccess;
            kernel_to_disassembly = std::move(dxr_kernel_to_disassembly);
            error_msg             = std::move(dxr_error_str);
        }
    }
    return status;
}

beKA::beStatus KcCliAmdgpuDisComputeStrategy::ParseKernels(const std::string&                  text_disassembly,
                                                           std::map<std::string, std::string>& kernel_to_disassembly,
                                                           std::string&                        error_msg) const
{
    beKA::beStatus status = beKA::beStatus::kBeStatusGeneralFailed;
    assert(!text_disassembly.empty());
    if (!text_disassembly.empty())
    {
        // Get to the .text section.
        size_t curr_pos = text_disassembly.find(kAmdgpuDisDotTextToken);
        assert(curr_pos != std::string::npos);
        if (curr_pos != std::string::npos)
        {
            // These will be used to extract the disassembly for each shader.
            size_t kernel_offset_begin = 0;
            size_t kernel_offset_end   = 0;

            // Parse the .text section. Identify each shader's area by its ".size" token.
            curr_pos = text_disassembly.find(kAmdgpuDisDotSizeToken);
            if (curr_pos == std::string::npos)
            {
                error_msg = kStrErrorNoShaderFoundInDisassembly;
            }

            while (curr_pos != std::string::npos)
            {
                bool is_kernel_name_in_quotes = false;

                // Find the name of the kernel.
                std::string kernel_name;
                size_t      end_of_line = text_disassembly.find("\n", curr_pos);
                std::string line        = text_disassembly.substr(curr_pos, end_of_line - curr_pos);
                auto        found       = line.find(kAmdgpuDisKernelNameStartToken);
                if (found != std::string::npos)
                {
                    size_t start = found + strlen(kAmdgpuDisKernelNameStartToken);
                    kernel_name  = line.substr(start, end_of_line - start);
                    curr_pos     = end_of_line;
                }
                else
                {
                    found = line.find(kAmdgpuDisKernelNameStartTokenWithQuotes);
                    if (found != std::string::npos)
                    {
                        is_kernel_name_in_quotes = true;
                        size_t start             = found + strlen(kAmdgpuDisKernelNameStartTokenWithQuotes);
                        kernel_name              = line.substr(start, end_of_line - start);
                        kernel_name              = BeMangledKernelUtils::UnQuote(kernel_name);
                        curr_pos                 = end_of_line;
                    }
                }

                if (!kernel_name.empty() && curr_pos != std::string::npos)
                {
                    // Construct the shader end token "kernel_name_symend:".
                    std::stringstream kernel_end_token_stream;
                    if (is_kernel_name_in_quotes)
                    {
                        kernel_end_token_stream << BeMangledKernelUtils::Quote(kernel_name + kAmdgpuDisShaderEndTokenNoColon) << ":";
                    }
                    else
                    {
                        kernel_end_token_stream << kernel_name << kAmdgpuDisShaderEndToken;
                    }
                    std::string kernel_token_end = kernel_end_token_stream.str();

                    // Extract the kernel disassembly.
                    kernel_offset_begin            = curr_pos;
                    kernel_offset_end              = text_disassembly.find(kernel_token_end, kernel_offset_begin);
                    std::string kernel_disassembly = text_disassembly.substr(curr_pos, kernel_offset_end - curr_pos);
                    kernel_disassembly = BeUtils::TrimLeadingAndTrailingWhitespace(kernel_disassembly);
                    kernel_name                        = BeMangledKernelUtils::DemangleShaderName(kernel_name);
                    kernel_to_disassembly[kernel_name] = KcUtilsLightning::PrefixWithISAHeader(kernel_name, kernel_disassembly);
                }
                else
                {
                    error_msg         = kStrErrorCannotParseDisassembly;
                    status            = beKA::beStatus::kBeStatusCannotParseDisassemblyGeneral;
                    kernel_offset_end = found;
                }

                // Look for the next shader.
                curr_pos = text_disassembly.find(kAmdgpuDisDotSizeToken, kernel_offset_end);
            }
        }
        else
        {
            error_msg = kStrErrorCannotParseDisassembly;
        }
    }

    if (!kernel_to_disassembly.empty())
    {
        status = beKA::beStatus::kBeStatusSuccess;
    }
    return status;
}

// Source-path post-processing helpers.

class SourceCodeLocator
{
public:
    struct Result
    {
        bool        found = false;
        std::string found_path;
    };

    explicit SourceCodeLocator(std::vector<std::string>            source_dirs,
                               std::vector<SubstituteSourcePath>   substitute_paths = {})
        : source_dirs_(std::move(source_dirs))
        , substitute_paths_(std::move(substitute_paths))
    {
    }

    Result FindSourceFile(const std::string& missing_file) const
    {
        Result result;
        if (missing_file.empty())
        {
            return result;
        }

        std::string base = Basename(missing_file);

        for (const auto& sd_str : source_dirs_)
        {
            std::string candidate1 = JoinPath(sd_str, missing_file);
            if (BeUtils::IsFilePresent(candidate1))
            {
                result.found      = true;
                result.found_path = ToAbsoluteNormalized(candidate1);
                return result;
            }

            std::string candidate2 = JoinPath(sd_str, base);
            if (BeUtils::IsFilePresent(candidate2))
            {
                result.found      = true;
                result.found_path = ToAbsoluteNormalized(candidate2);
                return result;
            }
        }

        for (const auto& [from, to] : substitute_paths_)
        {
            std::string norm_missing = NormalizeToForwardSlash(missing_file);
            std::string norm_from    = NormalizeToForwardSlash(from);

            if (norm_missing.size() >= norm_from.size() &&
                norm_missing.compare(0, norm_from.size(), norm_from) == 0)
            {
                std::string remainder = BeUtils::NormalizePathSeparatorsToCurrentOS(missing_file.substr(from.size()));
                std::string candidate = to + remainder;
                if (BeUtils::IsFilePresent(candidate))
                {
                    result.found      = true;
                    result.found_path = ToAbsoluteNormalized(candidate);
                    return result;
                }
            }
        }

        return result;
    }

private:
    static std::string Basename(const std::string& path)
    {
        size_t pos = path.find_last_of("/\\");
        return (pos != std::string::npos) ? path.substr(pos + 1) : path;
    }

    static std::string JoinPath(const std::string& dir, const std::string& file_path)
    {
        std::string relative = file_path;

        // Strip Windows drive letter prefix on any platform (cross-compiled DWARF).
        if (relative.size() >= 3 && std::isalpha(static_cast<unsigned char>(relative[0])) &&
            relative[1] == ':' && (relative[2] == '\\' || relative[2] == '/'))
        {
            relative = relative.substr(3);
        }

        if (!relative.empty() && (relative[0] == '/' || relative[0] == '\\'))
        {
            relative = relative.substr(1);
        }

        relative = BeUtils::NormalizePathSeparatorsToCurrentOS(relative);

        std::string result = dir;
        if (!result.empty() && result.back() != '/' && result.back() != '\\')
        {
            result += '/';
        }
        result += relative;
        return result;
    }

    static std::string ToAbsoluteNormalized(const std::string& path)
    {
        osFilePath os_path;
        os_path.setFullPathFromString(gtString().fromASCIIString(path.c_str()));
        os_path.resolveToAbsolutePath();
        return std::string(os_path.asString().asASCIICharArray());
    }

    static std::string NormalizeToForwardSlash(const std::string& path)
    {
        std::string result = path;
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }

    std::vector<std::string>          source_dirs_;
    std::vector<SubstituteSourcePath> substitute_paths_;
};

std::string TrimStr(std::string input_string)
{
    static const char* kSpecialChars = " \t\n\r\f\v";

    input_string.erase(input_string.find_last_not_of(kSpecialChars) + 1);
    input_string.erase(0, input_string.find_first_not_of(kSpecialChars));

    return input_string;
}

static std::vector<std::string> ReadSourceFileLines(const std::string& file_path)
{
    std::vector<std::string> lines;
    std::ifstream            file(file_path);
    if (file.good())
    {
        std::string line;
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }
    }
    return lines;
}

std::string ReplaceMissingPaths(const std::string& disasm, const std::unordered_map<std::string, std::string>& path_map)
{
    std::unordered_map<std::string, std::vector<std::string>> file_cache;

    std::istringstream iss(disasm);
    std::ostringstream oss;
    std::string        line;

    while (std::getline(iss, line))
    {
        if (!line.empty() && line[0] == ';')
        {
            size_t colon = line.rfind(':');
            if (colon != std::string::npos)
            {
                std::string after_colon = line.substr(colon + 1);
                if (after_colon.empty() || !std::all_of(after_colon.begin(), after_colon.end(), ::isdigit))
                {
                    oss << line << '\n';
                    continue;
                }
                std::string path = TrimStr(line.substr(1, colon - 1));
                if (!path.empty() && path_map.count(path))
                {
                    const std::string& resolved_path = path_map.at(path);
                    int                line_num      = std::atoi(after_colon.c_str());

                    line = "; " + resolved_path + line.substr(colon);
                    oss << line << '\n';

                    auto cache_it = file_cache.find(resolved_path);
                    if (cache_it == file_cache.end())
                    {
                        cache_it = file_cache.emplace(resolved_path, ReadSourceFileLines(resolved_path)).first;
                    }
                    const auto& src_lines = cache_it->second;
                    if (line_num > 0 && static_cast<size_t>(line_num) <= src_lines.size())
                    {
                        oss << "; " << src_lines[line_num - 1] << '\n';
                    }
                    else
                    {
                        oss << ';' << " MISSING_SOURCE_CODE " << '\n';
                    }
                    continue;
                }
            }
        }
        oss << line << '\n';
    }

    return oss.str();
}

std::string ParseErrorStrForMissingSourceFiles(const std::string&                            error_str,
                                               const std::vector<std::string>&               source_code_dirs,
                                               const std::vector<SubstituteSourcePath>&      substitute_paths,
                                               std::unordered_map<std::string, std::string>& missing_files_map)
{
    const std::string kObjdumpStr      = "llvm-objdump";
    const std::string kWarningStr      = "warning:";
    const std::string kFailedSrcStr    = "failed to find source";
    const size_t      kWarningOffset   = kWarningStr.size();
    const size_t      kFailedSrcOffset = kFailedSrcStr.size();

    SourceCodeLocator  locator(source_code_dirs, substitute_paths);
    std::istringstream iss(error_str);
    std::stringstream  oss;
    std::string        line;
    while (std::getline(iss, line))
    {
        if (line.find(kObjdumpStr) == std::string::npos)
        {
            oss << line << '\n';
            continue;
        }

        auto warn_pos   = line.find(kWarningStr);
        auto failed_pos = line.find(kFailedSrcStr);

        if (warn_pos == std::string::npos || failed_pos == std::string::npos)
        {
            oss << line << '\n';
            continue;
        }

        std::string binary = TrimStr(line.substr(warn_pos + kWarningOffset, failed_pos - (warn_pos + kWarningOffset)));
        std::string source = TrimStr(line.substr(failed_pos + kFailedSrcOffset));
        if (!binary.empty() && !source.empty())
        {
            auto it = missing_files_map.find(source);
            if (it != missing_files_map.end())
            {
                continue;
            }
            else
            {
                auto res = locator.FindSourceFile(source);
                if (res.found)
                {
                    missing_files_map[source] = res.found_path;
                    continue;
                }
            }
        }

        oss << line << '\n';
    }

    return oss.str();
}

// Scan ISA text directly for unresolved source paths (fallback when llvm-objdump
// doesn't emit "failed to find source" stderr warnings, e.g. AmdPal pipelines).
void ScanDisassemblyForMissingPaths(const std::string&                            disasm,
                                    const std::vector<std::string>&               source_code_dirs,
                                    const std::vector<SubstituteSourcePath>&      substitute_paths,
                                    std::unordered_map<std::string, std::string>& missing_files_map)
{
    SourceCodeLocator  locator(source_code_dirs, substitute_paths);
    std::istringstream iss(disasm);
    std::string        line;

    std::unordered_set<std::string> checked_paths;

    while (std::getline(iss, line))
    {
        if (line.empty() || line[0] != ';')
        {
            continue;
        }

        size_t colon = line.rfind(':');
        if (colon == std::string::npos)
        {
            continue;
        }

        std::string after_colon = line.substr(colon + 1);
        if (after_colon.empty() || !std::all_of(after_colon.begin(), after_colon.end(), ::isdigit))
        {
            continue;
        }

        std::string path = TrimStr(line.substr(1, colon - 1));
        if (path.empty() || missing_files_map.count(path) || checked_paths.count(path))
        {
            continue;
        }
        checked_paths.insert(path);

        if (BeUtils::IsFilePresent(path))
        {
            continue;
        }

        auto res = locator.FindSourceFile(path);
        if (res.found)
        {
            missing_files_map[path] = res.found_path;
        }
    }
}

// ============================================================================

beKA::beStatus KcCliLlvmObjdumpComputeStrategy::Disassemble(const std::string&                       bin_file,
                                                            const std::vector<std::string>&          source_code_dirs,
                                                            const std::vector<SubstituteSourcePath>& substitute_paths,
                                                            bool,
                                                            std::string& text_disassembly)
{
    KcUtilsBinary::LogPreStep(kStrInfoDisassemblingBinary, bin_file);

    std::string    error_txt;
    beKA::beStatus ret = BeProgramBuilderLightning::DisassembleBinary(
        compiler_paths_.bin, bin_file, clang_device_, is_line_correlation_enabled_, verbose_,
        text_disassembly, error_txt, source_code_dirs, substitute_paths);

    if (text_disassembly.empty())
    {
        ret = beKA::beStatus::kBeStatusLightningObjDumpLaunchFailed;
    }

    if (ret == beKA::beStatus::kBeStatusSuccess && (!source_code_dirs.empty() || !substitute_paths.empty()))
    {
        std::unordered_map<std::string, std::string> missing_files_map;

        if (!error_txt.empty())
        {
            error_txt = ParseErrorStrForMissingSourceFiles(error_txt, source_code_dirs, substitute_paths, missing_files_map);
        }

        ScanDisassemblyForMissingPaths(text_disassembly, source_code_dirs, substitute_paths, missing_files_map);

        if (!missing_files_map.empty())
        {
            text_disassembly = ReplaceMissingPaths(text_disassembly, missing_files_map);
        }

        if (!error_txt.empty())
        {
            std::string remaining = TrimStr(error_txt);
            if (!remaining.empty())
            {
                KcUtilsBinary::LogErrorStatus(beKA::beStatus::kBeStatusLightningObjDumpLaunchWarning, error_txt);
            }
        }
        error_txt.clear();
    }
    else if (!error_txt.empty())
    {
        if (ret == beKA::beStatus::kBeStatusSuccess)
        {
            KcUtilsBinary::LogErrorStatus(beKA::beStatus::kBeStatusLightningObjDumpLaunchWarning, error_txt);
            error_txt.clear();
        }
        else
        {
            ret = beKA::beStatus::kBeStatusLightningObjDumpLaunchFailed;
        }
    }

    KcUtilsBinary::LogResult(ret == beKA::beStatus::kBeStatusSuccess);
    KcUtilsBinary::LogErrorStatus(ret, error_txt);
    return ret;
}

beKA::beStatus KcCliLlvmObjdumpComputeStrategy::ParseKernels(const std::string&                  text_disassembly,
                                                             std::map<std::string, std::string>& kernel_to_disassembly,
                                                             std::string&                        error_msg) const
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;
    // Replace labels of format "address   <label_name>:" with "label_name:"
    std::string new_isa_text = KcUtilsLightning::FormatLlvmIsaLabels(text_disassembly);

    // Split ISA text into per-kernel fragments.
    bool ret = KcUtilsLightning::SplitISAText(new_isa_text, amdhsa_kernels_md_.kernel_names, kernel_to_disassembly);

    // Eliminate the useless code.
    ret = ret && KcUtilsLightning::ReduceISA(binary_codeobj_file_, compiler_paths_, verbose_, kernel_to_disassembly);

    if (!ret)
    {
        status = beKA::beStatus::kBeStatusLightningSplitIsaFailed;
    }

    if (status != beKA::beStatus::kBeStatusSuccess)
    {
        error_msg = kStrErrorCannotParseDisassembly;
    }

    return status;
}

beKA::beStatus KcCliLlvmObjdumpGraphicsStrategy::ParseKernels(const std::string&                  text_disassembly,
                                                              std::map<std::string, std::string>& kernel_to_disassembly,
                                                              std::string&                        error_msg) const
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;
    // Replace labels of format "address   <label_name>:" with "label_name:"
    std::string new_isa_text = KcUtilsLightning::FormatLlvmIsaLabels(text_disassembly);

    auto kernel_names = beProgramBuilderBinary::GetKernelNames(amdpal_pipeline_md_);

    // Split ISA text into per-kernel fragments.
    bool ret = KcUtilsLightning::SplitISAText(new_isa_text, kernel_names, kernel_to_disassembly);

    // Eliminate the useless code.
    ret = ret && KcUtilsLightning::ReduceISA(binary_codeobj_file_, compiler_paths_, verbose_, kernel_to_disassembly);

    for (const auto& stage : amdpal_pipeline_md_.hardware_stages)
    {
        for (const auto& kernel : kernel_names)
        {
            if (kernel == stage.entry_point)
            {
                std::string stage_name = BeAmdPalMetaData::GetStageName(stage.stage_type);
                auto        beg        = stage_name.find(".");
                if (beg != std::string::npos)
                {
                    std::string hardware_stage = stage_name.substr(beg + 1);
                    auto        it             = kernel_to_disassembly.find(kernel);
                    if (it != kernel_to_disassembly.end())
                    {
                        kernel_to_disassembly[hardware_stage] = std::move(it->second);
                        kernel_to_disassembly.erase(it);
                    }
                }
            }
        }
    }

    if (!ret)
    {
        status = beKA::beStatus::kBeStatusLightningSplitIsaFailed;
    }

    if (status != beKA::beStatus::kBeStatusSuccess)
    {
        error_msg = kStrErrorCannotParseDisassembly;
    }

    return status;
}
