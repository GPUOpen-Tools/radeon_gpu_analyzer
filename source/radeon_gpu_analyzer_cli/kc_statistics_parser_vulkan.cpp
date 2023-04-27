//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <iosfwd>
#include <streambuf>
#include <vector>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include "radeon_gpu_analyzer_cli/kc_statistics_parser_vulkan.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"

// Constants.
static const char* kIsaSizeToken = "codeLenInByte";
static const char* kUsedVgprsToken = "NumVgprs";
static const char* kUsedSgprsToken = "NumSgprs";
static const char* kEndOfLineDelimiter = ";";
static const char* kOfflineDeviceNameTahiti = "tahiti";
static const char* kOfflineDeviceNameTonga = "tonga";
static const char* kOfflineDeviceNameCapeverde = "capeverde";
static const char* kOfflineDeviceNameHainan = "hainan";
static const char* kOfflineDeviceNameOland = "oland";
static const char* kOfflineDeviceNamePitcairn = "pitcairn";
static const char* kOfflineDeviceNameIceland = "iceland";

KcVulkanStatisticsParser::KcVulkanStatisticsParser()
{
}

KcVulkanStatisticsParser::~KcVulkanStatisticsParser()
{
}

/// Extracts a numeric value from the SC's textual statistics for Vulkan.
/// Params:
///     fileContent: the content of the SC statistics file.
///     attributeToken: the attribute whose value is to be extracted.
//      numericValue: the extracted value.
// Returns: true for success, false otherwise.
static bool ExtractNumericStatistic(const std::string& file_content, const char* attribute_token, size_t& extracted_value)
{
    bool ret = false;
    size_t value_begin_index = file_content.find(attribute_token);
    if (value_begin_index != std::string::npos)
    {
        value_begin_index += strlen(attribute_token) + 1;
        if (value_begin_index < file_content.size())
        {
            size_t value_end_index = file_content.find(kEndOfLineDelimiter, value_begin_index) - 1;
            if (value_end_index != std::string::npos)
            {
                size_t value_length = value_end_index - value_begin_index + 1;
                if (value_length > 0)
                {
                    // Extract the value.
                    std::string value = file_content.substr(value_begin_index, value_length);
                    std::string::iterator end_pos = std::remove_if(value.begin(),
                        value.end(), [&value](char c) { return (c == ' ' || !std::isdigit(c)); });
                    value.erase(end_pos, value.end());
                    try
                    {
                        extracted_value = std::stoi(value);
                        ret = true;
                    }
                    catch (...){}
                }
            }
        }
    }
    return ret;
}

// Extracts the ISA size in bytes.
static bool ExtractIsaSize(const std::string& file_content, size_t& isa_size_bytes)
{
    return ExtractNumericStatistic(file_content, kIsaSizeToken, isa_size_bytes);
}

// Extracts the number of used SGPRs.
static bool ExtractUsedSgprs(const std::string& file_content, size_t& isa_size_bytes)
{
    return ExtractNumericStatistic(file_content, kUsedSgprsToken, isa_size_bytes);
}

// Extracts the number of used VGPRs.
static bool ExtractUsedVgprs(const std::string& file_content, size_t& isa_sizeIn_bytes)
{
    return ExtractNumericStatistic(file_content, kUsedVgprsToken, isa_sizeIn_bytes);
}

static bool IsGfx6Device(const std::string& device)
{
    std::string device_lower = device;
    std::transform(device_lower.begin(), device_lower.end(), device_lower.begin(), ::tolower);
    bool ret = device_lower.find(kOfflineDeviceNameTahiti) != std::string::npos ||
        device_lower.find(kOfflineDeviceNameCapeverde) != std::string::npos ||
        device_lower.find(kOfflineDeviceNameHainan) != std::string::npos ||
        device_lower.find(kOfflineDeviceNameOland) != std::string::npos ||
        device_lower.find(kOfflineDeviceNamePitcairn) != std::string::npos;
    return ret;
}

bool KcVulkanStatisticsParser::ParseStatistics(const std::string& device, const gtString& statistics_file, beKA::AnalysisData& statistics)
{
    bool ret = false;
    statistics.isa_size = 0;
    statistics.num_sgprs_used = 0;
    statistics.num_vgprs_used = 0;
    statistics.num_vgprs_available = 256;
    statistics.num_sgprs_available = 106;
    statistics.lds_size_available = 65536;

    // Special cases.
    std::string device_lower = device;
    std::transform(device_lower.begin(), device_lower.end(), device_lower.begin(), ::tolower);
    if (device_lower.find(kOfflineDeviceNameTonga) != std::string::npos ||
        device_lower.find(kOfflineDeviceNameIceland) != std::string::npos)
    {
        statistics.num_sgprs_available = 96;
    }
    if (IsGfx6Device(device))
    {
        statistics.lds_size_available = 32768;
    }

    // Check if the file exists.
    if (!statistics_file.isEmpty())
    {
        osFilePath file_path(statistics_file);
        if (file_path.exists())
        {
            std::ifstream file(statistics_file.asASCIICharArray());
            std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            if (!file_content.empty())
            {
                // Extract the ISA size in bytes.
                size_t isa_size_bytes = 0;
                bool is_isa_size_extracted = ExtractIsaSize(file_content, isa_size_bytes);
                if (is_isa_size_extracted)
                {
                    statistics.isa_size = isa_size_bytes;
                }

                // Extract the number of used SGPRs.
                size_t used_sgprs = 0;
                bool is_sgprs_extracted = ExtractUsedSgprs(file_content, used_sgprs);

                if (is_sgprs_extracted)
                {
                    statistics.num_sgprs_used = used_sgprs;
                }

                // Extract the number of used VGPRs.
                size_t used_vgprs = 0;
                bool is_vgprs_extracted = ExtractUsedVgprs(file_content, used_vgprs);
                if (is_vgprs_extracted)
                {
                    statistics.num_vgprs_used = used_vgprs;
                }

                // We succeeded if all data was extracted successfully.
                ret = (is_isa_size_extracted && is_sgprs_extracted && is_vgprs_extracted);
            }
        }
    }

    return ret;
}
