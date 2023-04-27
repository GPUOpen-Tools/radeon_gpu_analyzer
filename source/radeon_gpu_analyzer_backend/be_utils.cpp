//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cctype>
#include <cwctype>

// Infra.
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4309)
#endif
#include "external/amdt_base_tools/Include/gtAssert.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osFile.h"
#include "external/amdt_os_wrappers/Include/osApplication.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

// Local.
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_string_constants.h"
#include "DeviceInfoUtils.h"

// CLI.
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"

// Local constants and definitions.
static const char* kShaderStageVs = "vs";
static const char* kShaderStageHs = "hs";
static const char* kShaderStageDs = "ds";
static const char* kShaderStageGs = "gs";
static const char* kShaderStagePs = "ps";
static const char* kShaderStageCs = "cs";

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***

// Converts string to its lower-case version.
static std::string ToLower(const std::string& str)
{
    std::string lstr = str;
    std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](const char& c) {return std::tolower(c); });
    return lstr;
}

// Retrieves the list of devices according to the given HW generation.
static void AddGenerationDevices(GDT_HW_GENERATION hw_generation, std::vector<GDT_GfxCardInfo>& card_list,
    std::set<std::string> &public_device_unique_names, bool should_convert_to_lower = false)
{
    std::vector<GDT_GfxCardInfo> card_list_buffer;
    if (AMDTDeviceInfoUtils::Instance()->GetAllCardsInHardwareGeneration(hw_generation, card_list_buffer))
    {
        card_list.insert(card_list.end(), card_list_buffer.begin(), card_list_buffer.end());

        for (const GDT_GfxCardInfo& cardInfo : card_list_buffer)
        {
            public_device_unique_names.insert(should_convert_to_lower ? ToLower(cardInfo.m_szCALName) : cardInfo.m_szCALName);
        }
    }
}

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***

bool BeUtils::GdtHwGenToNumericValue(GDT_HW_GENERATION hw_generation, size_t& gfx_ip)
{
    const size_t kGfxIp9 = 9;
    const size_t kGfxIp10 = 10;

    bool ret = true;

    switch (hw_generation)
    {
    case GDT_HW_GENERATION_GFX9:
        gfx_ip = kGfxIp9;
        break;

    case GDT_HW_GENERATION_GFX10:
        gfx_ip = kGfxIp10;
        break;

    case GDT_HW_GENERATION_GFX103:
        gfx_ip = kGfxIp10;
        break;

    case GDT_HW_GENERATION_NONE:
    case GDT_HW_GENERATION_NVIDIA:
    case GDT_HW_GENERATION_LAST:
    default:
        // We should not get here.
        GT_ASSERT_EX(false, L"Unsupported HW Generation.");
        ret = false;
        break;
    }

    return ret;
}

bool BeUtils::GetAllGraphicsCards(std::vector<GDT_GfxCardInfo>& card_list,
    std::set<std::string>& public_device_unique_names,
    bool convert_to_lower /*= false*/)
{
    // Retrieve the list of devices for every relevant hardware generations.
    AddGenerationDevices(GDT_HW_GENERATION_GFX9, card_list, public_device_unique_names, convert_to_lower);
    AddGenerationDevices(GDT_HW_GENERATION_GFX10, card_list, public_device_unique_names, convert_to_lower);
    AddGenerationDevices(GDT_HW_GENERATION_GFX103, card_list, public_device_unique_names, convert_to_lower);
    AddGenerationDevices(GDT_HW_GENERATION_GFX11, card_list, public_device_unique_names, convert_to_lower);

    return (!card_list.empty() && !public_device_unique_names.empty());
}

bool BeUtils::GetPreRdna3GraphicsCards(std::vector<GDT_GfxCardInfo>& card_list,
                                   std::set<std::string>&        public_device_unique_names,
                                   bool                          should_convert_to_lower /*= false*/)
{
    // Retrieve the list of devices for every relevant hardware generations.
    AddGenerationDevices(GDT_HW_GENERATION_GFX9, card_list, public_device_unique_names, should_convert_to_lower);
    AddGenerationDevices(GDT_HW_GENERATION_GFX10, card_list, public_device_unique_names, should_convert_to_lower);
    AddGenerationDevices(GDT_HW_GENERATION_GFX103, card_list, public_device_unique_names, should_convert_to_lower);

    return (!card_list.empty() && !public_device_unique_names.empty());
}

bool BeUtils::GetMarketingNameToCodenameMapping(std::map<std::string, std::set<std::string>>& cards_map)
{
    std::vector<GDT_GfxCardInfo> card_list;
    std::set<std::string> unique_names;

    // Retrieve the list of all supported cards.
    bool ret = GetAllGraphicsCards(card_list, unique_names);

    if (ret)
    {
        for (const GDT_GfxCardInfo& card : card_list)
        {
            if (card.m_szMarketingName != nullptr &&
                card.m_szCALName != nullptr &&
                (strlen(card.m_szMarketingName) > 1) &&
                (strlen(card.m_szCALName) > 1))
            {
                // Create the key string.
                std::string display_name;
                ret = AMDTDeviceInfoUtils::Instance()->GetHardwareGenerationDisplayName(card.m_generation, display_name);
                if (ret)
                {
                    const std::string kMi100 = "gfx908";
                    const std::string kGenerationCdna = "CDNA";
                    std::stringstream name_builder;
                    name_builder << card.m_szCALName;
                    if (kMi100.compare(card.m_szCALName) != 0)
                    {
                        name_builder << " (" << display_name << ")";
                    }
                    else
                    {
                        // Special case for MI100.
                        name_builder << " (" << kGenerationCdna << ")";
                    }

                    // Add this item to the relevant container in the map.
                    cards_map[name_builder.str()].insert(card.m_szMarketingName);
                }
            }
        }
    }

    return ret;
}

void BeUtils::DeleteOutputFiles(const BeProgramPipeline& output_file_path)
{
    DeleteFileFromDisk(output_file_path.vertex_shader);
    DeleteFileFromDisk(output_file_path.tessellation_control_shader);
    DeleteFileFromDisk(output_file_path.tessellation_evaluation_shader);
    DeleteFileFromDisk(output_file_path.geometry_shader);
    DeleteFileFromDisk(output_file_path.fragment_shader);
    DeleteFileFromDisk(output_file_path.compute_shader);
}

void BeUtils::DeleteFileFromDisk(const gtString& file_path)
{
    osFilePath os_path(file_path);
    if (os_path.exists())
    {
        osFile osFile(os_path);
        osFile.deleteFile();
    }
}

void BeUtils::DeleteFileFromDisk(const std::string& file_path)
{
    gtString path_gtstr;
    path_gtstr << file_path.c_str();
    return DeleteFileFromDisk(path_gtstr);
}

bool BeUtils::IsFilePresent(const std::string& filename)
{
    bool  ret = true;
    if (!filename.empty())
    {
        std::ifstream file(filename);
        ret = (file.good() && file.peek() != std::ifstream::traits_type::eof());
    }
    return ret;
}

std::string BeUtils::GetFileExtension(const std::string& filename)
{
    size_t offset = filename.rfind('.');
    const std::string& ext = (offset != std::string::npos && ++offset < filename.size()) ? filename.substr(offset) : "";
    return ext;
}

bool BeUtils::ReadBinaryFile(const std::string& filename, std::vector<char>& content)
{
    bool ret = false;
    std::ifstream input;
    input.open(filename.c_str(), std::ios::binary);

    if (input.is_open())
    {
        content = std::vector<char>(std::istreambuf_iterator<char>(input), {});
        ret = !content.empty();
    }
    return ret;
}

bool BeUtils::IsFilesIdentical(const std::string& filename1, const std::string& filename2)
{
    std::vector<char> content1;
    std::vector<char> content2;
    bool is_file_read1 = BeUtils::ReadBinaryFile(filename1, content1);
    bool is_file_read2 = BeUtils::ReadBinaryFile(filename2, content2);
    return (is_file_read1 && is_file_read2 && std::equal(content1.begin(), content1.end(), content2.begin()));
}

void BeUtils::PrintCmdLine(const std::string & cmd_line, bool should_print_cmd)
{
    const char* kBeStrLaunchingExternalProcess = "Launching external process: \n";
    if (should_print_cmd)
    {
        std::cout << std::endl << kBeStrLaunchingExternalProcess << cmd_line << std::endl << std::endl;
    }
}

void BeUtils::SplitString(const std::string& str, char delim, std::vector<std::string>& dst)
{
    std::stringstream ss;
    ss.str(str);
    std::string substr;
    while (std::getline(ss, substr, delim))
    {
        dst.push_back(substr);
    }
}

static void LeftTrim(const std::string& text, std::string& trimmed_text)
{
    trimmed_text    = text;
    auto space_iter = std::find_if(trimmed_text.begin(), trimmed_text.end(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    trimmed_text.erase(trimmed_text.begin(), space_iter);
}

static void RightTrim(const std::string& text, std::string& trimmed_text)
{
    trimmed_text    = text;
    auto space_iter = std::find_if(trimmed_text.rbegin(), trimmed_text.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    trimmed_text.erase(space_iter.base(), trimmed_text.end());
}

void BeUtils::TrimLeadingAndTrailingWhitespace(const std::string& text, std::string& trimmed_text)
{
    // Trim the whitespace off the left and right sides of the incoming text.
    std::string left_trimmed;
    LeftTrim(text, left_trimmed);
    RightTrim(left_trimmed, trimmed_text);
}

bool BeUtils::DeviceNameLessThan(const std::string& a, const std::string& b)
{
    const char* kGfxNotationToken = "gfx";
    bool ret = true;
    size_t size_a = a.find(kGfxNotationToken);
    size_t size_b = b.find(kGfxNotationToken);
    if (size_a == std::string::npos && size_b == std::string::npos)
    {
        // Neither name is in gfx-notation, compare using standard string logic.
        ret = a.compare(b) < 0;
    }
    else if (!(size_a != std::string::npos && size_b != std::string::npos))
    {
        // Only one name has the gfx notation, assume that it is a newer generation.
        ret = (size_b != std::string::npos);
    }
    else
    {
        // Both names are in gfx notation, compare according to the number.
        std::vector<std::string> split1;
        std::vector<std::string> split2;
        BeUtils::SplitString(a, 'x', split1);
        BeUtils::SplitString(b, 'x', split2);
        assert(split1.size() > 1);
        assert(split2.size() > 1);
        if (split1.size() > 1 && split2.size() > 1)
        {
            try
            {
                int num1 = std::stoi(split1[1], nullptr, 16);
                int num2 = std::stoi(split2[1], nullptr, 16);
                ret = ((num2 - num1) > 0);
            }
            catch (...)
            {
                ret = false;
            }
        }
    }
    return ret;
}

bool BeUtils::DisassembleCodeObject(const std::string& code_object_filename, bool should_print_cmd,
    std::string& disassembly_whole, std::string& disassembly_text, std::string& error_msg)
{
    static const wchar_t* kLcDisassemblerExe = L"amdgpu-dis";
    static const wchar_t* kLcDisassemblerDir = L"utils/lc/disassembler";
    const char* kStrErrorCodeObjectParseFailure = "Error: failed to parse Code Object .text section.";
    const char* kStrErrorLcDisassemblerLaunchFailure = "Error: failed to launch the LC disassembler.";

    // Build the command.
    std::stringstream cmd;
    cmd << code_object_filename;

    osFilePath lc_disassembler_exe;
    long exit_code = 0;

    osGetCurrentApplicationPath(lc_disassembler_exe, false);
    lc_disassembler_exe.appendSubDirectory(kLcDisassemblerDir);
    lc_disassembler_exe.setFileName(kLcDisassemblerExe);

    // Clear the error message buffer.
    error_msg.clear();
    std::string out_text;

    KcUtils::ProcessStatus status = KcUtils::LaunchProcess(lc_disassembler_exe.asString().asASCIICharArray(),
        cmd.str(),
        "",
        kProcessWaitInfinite,
        should_print_cmd,
        out_text,
        error_msg,
        exit_code);

    // Extract the .text disassembly.
    assert(!out_text.empty());
    disassembly_whole = out_text;

    if (!disassembly_whole.empty())
    {
        // Find where the .text section starts.
        size_t text_offset_start = disassembly_whole.find(".text");
        assert(text_offset_start != std::string::npos);
        assert(text_offset_start != std::string::npos &&
            text_offset_start < disassembly_whole.size() + 5);
        if (text_offset_start < disassembly_whole.size() + 5)
        {
            // Skip .text identifier.
            text_offset_start += 5;

            // Find where the relevant portion of the disassembly ends.
            size_t text_offset_end = disassembly_whole.find("s_code_end");
            if (text_offset_end == std::string::npos)
            {
                // If s_code_end could not be found, compromise on finding the beginning of the next section.
                text_offset_end = disassembly_whole.find(".section", text_offset_start);
            }
            assert(text_offset_end != std::string::npos);
            if (text_offset_end != std::string::npos)
            {
                // Extract the relevant section.
                size_t num_characters = text_offset_end - text_offset_start;
                assert(num_characters > 0);
                assert(num_characters < disassembly_whole.size() - text_offset_start);
                if (num_characters > 0 && num_characters < disassembly_whole.size() - text_offset_start)
                {
                    disassembly_text = disassembly_whole.substr(text_offset_start, num_characters);
                }
                else if (error_msg.empty())
                {
                    error_msg = kStrErrorCodeObjectParseFailure;
                }
            }
            else if (error_msg.empty())
            {
                error_msg = kStrErrorCodeObjectParseFailure;
            }
        }
    }

    if (disassembly_text.empty())
    {
        if (status == KcUtils::ProcessStatus::kSuccess && error_msg.empty())
        {
            error_msg = kStrErrorLcDisassemblerLaunchFailure;
        }
    }

    return (status == KcUtils::ProcessStatus::kSuccess ? kBeStatusSuccess : kBeStatusdx12BackendLaunchFailure);
}

static bool ExtractAttributeValue(const std::string &disassembly_whole, size_t kd_pos, const std::string& attribute_name, uint32_t& value)
{
    bool ret = false;
    bool should_abort = false;
    bool is_before = false;
    try
    {
        // Offset where our attribute is within the string.
        size_t start_pos_temp = 0;

        // The reference symbol.
        const std::string kKdSymbolToken = ".symbol:";
        if (attribute_name < kKdSymbolToken)
        {
            // Look before the reference symbol.
           start_pos_temp = disassembly_whole.rfind(attribute_name, kd_pos);
           is_before = true;
        }
        else if (attribute_name > kKdSymbolToken)
        {
            // Look after the reference symbol.
            start_pos_temp = disassembly_whole.find(attribute_name, kd_pos);
        }
        else
        {
            // We shouldn't get here.
            assert(false);
            should_abort = true;
        }

        if (!should_abort)
        {
            start_pos_temp += attribute_name.size();
            assert((is_before && start_pos_temp < kd_pos) || (!is_before && start_pos_temp > kd_pos));
            if ((is_before && start_pos_temp < kd_pos) || (!is_before && start_pos_temp > kd_pos))
            {
                while (std::iswspace(disassembly_whole[++start_pos_temp]));
                assert((is_before && start_pos_temp < kd_pos) || (!is_before && start_pos_temp > kd_pos));
                if ((is_before && start_pos_temp < kd_pos) || (!is_before && start_pos_temp > kd_pos))
                {
                    size_t endPos = start_pos_temp;
                    while (!std::iswspace(disassembly_whole[++endPos]));
                    assert(start_pos_temp < endPos);
                    if (start_pos_temp < endPos)
                    {
                        // Extract the string representing the value and convert to non-negative decimal number.
                        std::string value_as_str = disassembly_whole.substr(start_pos_temp, endPos - start_pos_temp);
                        std::stringstream conversion_stream;
                        conversion_stream << std::hex << value_as_str;
                        conversion_stream >> value;
                        ret = true;
                    }
                }
            }
        }
    }
    catch (...)
    {
        // Failure occurred.
        ret = false;
    }
    return ret;
}

bool BeUtils::ExtractCodeObjectStatistics(const std::string& disassembly_whole,
    std::map<std::string, beKA::AnalysisData>& data_map)
{
    bool ret = false;
    data_map.clear();

    const char* kKernelSymbolToken = ".kd";
    size_t start_pos = disassembly_whole.find(kKernelSymbolToken);
    while (start_pos != std::string::npos)
    {
        // Extract the kernel name.
        std::string kernel_name;
        size_t start_pos_temp = start_pos;
        std::stringstream kernel_name_stream;
        while (--start_pos_temp > 0 && !std::iswspace(disassembly_whole[start_pos_temp]));
        assert(start_pos_temp + 1 < start_pos - 1);
        if (start_pos_temp + 1 < start_pos - 1)
        {
            kernel_name = disassembly_whole.substr(start_pos_temp + 1, start_pos - start_pos_temp - 1);
            auto iter = data_map.find(kernel_name);
            assert(iter == data_map.end());
            if (iter == data_map.end())
            {
                // LDS.
                const std::string kLdsUsageToken = ".group_segment_fixed_size:";
                uint32_t lds_usage = 0;
                bool is_ok = ExtractAttributeValue(disassembly_whole, start_pos, kLdsUsageToken, lds_usage);
                assert(is_ok);

                // SGPR count.
                const std::string kSgprToken = ".sgpr_count:";
                uint32_t sgpr_count = 0;
                is_ok = ExtractAttributeValue(disassembly_whole, start_pos, kSgprToken, sgpr_count);
                assert(is_ok);

                // SGPR spill count.
                const std::string kSgprSpillCountToken = ".sgpr_spill_count:";
                uint32_t sgpr_spill_count = 0;
                is_ok = ExtractAttributeValue(disassembly_whole, start_pos, kSgprSpillCountToken, sgpr_spill_count);
                assert(is_ok);

                // VGPR count.
                const std::string kVgprCountToken = ".vgpr_count:";
                uint32_t vgpr_count = 0;
                is_ok = ExtractAttributeValue(disassembly_whole, start_pos, kVgprCountToken, vgpr_count);
                assert(is_ok);

                // VGPR spill count.
                const std::string kVgprSpillCountToken  = ".vgpr_spill_count";
                uint32_t vgpr_spill_count = 0;
                is_ok = ExtractAttributeValue(disassembly_whole, start_pos, kVgprSpillCountToken , vgpr_spill_count);
                assert(is_ok);

                // Wavefront size.
                const std::string kWavefrontSizeToken = ".wavefront_size:";
                uint32_t wavefront_size = 0;
                is_ok = ExtractAttributeValue(disassembly_whole, start_pos, kWavefrontSizeToken, wavefront_size);
                assert(is_ok);

                // Add values which were extracted from the Code Object meta data.
                AnalysisData data;
                data.lds_size_used = lds_usage;
                data.num_sgprs_used = sgpr_count;
                data.num_sgpr_spills = sgpr_spill_count;
                data.num_vgprs_used = vgpr_count;
                data.num_vgpr_spills = vgpr_spill_count;
                data.wavefront_size = wavefront_size;

                // Add fixed values.
                data.lds_size_available = 65536;
                data.num_vgprs_available = 256;
                data.num_sgprs_available = 106;

                // Add the kernel's stats to the map.
                data_map[kernel_name] = data;

                // Move to the next kernel.
                start_pos = disassembly_whole.find(kKernelSymbolToken, start_pos + 1);
            }
        }
    }

    ret = !data_map.empty();
    return ret;
}

bool BeUtils::IsValidAmdgpuShaderStage(const std::string& shader_stage)
{
    bool ret = (shader_stage.compare(kShaderStageCs) == 0) || (shader_stage.compare(kShaderStagePs) == 0) || (shader_stage.compare(kShaderStageGs) == 0) ||
               (shader_stage.compare(kShaderStageVs) == 0) || (shader_stage.compare(kShaderStageHs) == 0) || (shader_stage.compare(kShaderStageDs) == 0);
    return ret;
}

bool BeUtils::BePipelineStageToAmdgpudisStageName(BePipelineStage pipeline_stage, std::string& amdgpu_dis_stage)
{
    bool ret = true;
    switch (pipeline_stage)
    {
    case kVertex:
        amdgpu_dis_stage = kShaderStageVs;
        break;
    case kTessellationControl:
        amdgpu_dis_stage = kShaderStageHs;
        break;
    case kTessellationEvaluation:
        amdgpu_dis_stage = kShaderStageDs;
        break;
    case kGeometry:
        amdgpu_dis_stage = kShaderStageGs;
        break;
    case kFragment:
        amdgpu_dis_stage = kShaderStagePs;
        break;
    case kCompute:
        amdgpu_dis_stage = kShaderStageCs;
        break;
    case kCount:
    default:
        // We shouldn't get here.
        ret = false;
        assert(ret);
        break;
    }
    return ret;
}
