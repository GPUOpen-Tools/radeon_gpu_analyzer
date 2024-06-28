//======================================================================
// Copyright 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

// Infra.
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_statistics_parser_opengl.h"

// Constants.
const char* kGlIsaSizeToken = "codeLenInByte";
const char* kGlUsedVgprsToken = "NumVgprs";
const char* kGlUsedSgprsToken = "NumSgprs";
const char* kGlUsedScratchSizeToken = "ScratchSize";
const char* kGlLDSByteSizeToken = "LDSByteSize";
const char* kGlEndOfLineDelimiter = "\n";

// Extracts a numeric value from the SC's textual statistics for Vulkan.
// Params:
//     fileContent: the content of the SC statistics file.
//     attributeToken: the attribute whose value is to be extracted.
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
            size_t value_end_index = file_content.find(kGlEndOfLineDelimiter, value_begin_index) - 1;
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
                    if (value.empty() == false)
                    {
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
    }
    return ret;
}

// Extracts the ISA size in bytes.
static bool ExtractIsaSize(const std::string& file_content, size_t& isa_size_bytes)
{
    return ExtractNumericStatistic(file_content, kGlIsaSizeToken, isa_size_bytes);
}

// Extracts the number of used SGPRs.
static bool ExtractUsedSgprsGL(const std::string& file_content, size_t& used_sgprs)
{
    return ExtractNumericStatistic(file_content, kGlUsedSgprsToken, used_sgprs);
}

// Extracts the number of used VGPRs.
static bool ExtractUsedVgprsGL(const std::string& file_content, size_t& used_vgprs)
{
    return ExtractNumericStatistic(file_content, kGlUsedVgprsToken, used_vgprs);
}

// Extracts the scratch registers attribute.
static bool ExtractScratchSizeGL(const std::string& file_content, size_t& scratch_regs)
{
    return ExtractNumericStatistic(file_content, kGlUsedScratchSizeToken, scratch_regs);
}

// Extracts the LDS byte size attribute.
static bool ExtractLdsByteSizeGL(const std::string& file_content, size_t& scratch_regs)
{
    return ExtractNumericStatistic(file_content, kGlLDSByteSizeToken, scratch_regs);
}

bool KcOpenGLStatisticsParser::ParseStatistics(const std::string&, const gtString& stats_file, beKA::AnalysisData& stats)
{
    bool ret = false;
    stats.isa_size = 0;
    stats.num_sgprs_used = 0;
    stats.num_vgprs_used = 0;
    stats.scratch_memory_used = 0;
    stats.lds_size_used = 0;

    // Check if the file exists.
    if (!stats_file.isEmpty())
    {
        osFilePath file_path(stats_file);
        if (file_path.exists())
        {
            std::ifstream file(stats_file.asASCIICharArray());
            std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            if (!file_content.empty())
            {
                // Extract the ISA size in bytes.
                size_t isa_size_bytes = 0;
                bool is_isa_size_extracted = ExtractIsaSize(file_content, isa_size_bytes);
                if (is_isa_size_extracted)
                {
                    stats.isa_size = isa_size_bytes;
                }

                // Extract the number of used SGPRs.
                size_t used_sgprs = 0;
                bool is_sgprs_extracted = ExtractUsedSgprsGL(file_content, used_sgprs);
                if (is_sgprs_extracted)
                {
                    stats.num_sgprs_used = used_sgprs;
                }

                // Extract the number of used VGPRs.
                size_t used_vgprs = 0;
                bool is_vgprs_extracted = ExtractUsedVgprsGL(file_content, used_vgprs);
                if (is_vgprs_extracted)
                {
                    stats.num_vgprs_used = used_vgprs;
                }

                // Extract the scratch registers size.
                size_t scratch_regs = 0;
                bool is_scratch_regs_extracted = ExtractScratchSizeGL(file_content, scratch_regs);
                if (is_scratch_regs_extracted)
                {
                    stats.scratch_memory_used = scratch_regs;
                }

                // Extract the LDS Byte size.
                size_t lds_byte_size = 0;
                bool   is_lds_byte_size_extracted = ExtractLdsByteSizeGL(file_content, lds_byte_size);
                if (is_scratch_regs_extracted)
                {
                    stats.lds_size_used = lds_byte_size;
                }

                // We succeeded if all data was extracted successfully.
                ret = (is_isa_size_extracted && is_sgprs_extracted && is_vgprs_extracted && is_scratch_regs_extracted && is_lds_byte_size_extracted);
            }
        }
    }

    return ret;
}
