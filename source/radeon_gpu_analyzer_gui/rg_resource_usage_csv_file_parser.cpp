// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/rg_resource_usage_csv_file_parser.h"

bool RgResourceUsageCsvFileParser::ProcessLineTokens(const std::vector<std::string>& tokens)
{
    bool ret = false;

    // Verify that the number of tokens matches the number of columns in the file being parsed.
    int num_tokens = static_cast<int>(tokens.size());
    assert(num_tokens == rgResourceUsageCsvFileColumns::kCount);
    if (num_tokens == rgResourceUsageCsvFileColumns::kCount)
    {
        // Extract all info from the file's line tokens.
        resource_usage_data_.device                = tokens[rgResourceUsageCsvFileColumns::kDevice];
        resource_usage_data_.scratch_memory         = std::atoi(tokens[rgResourceUsageCsvFileColumns::kScratchMemory].c_str());
        resource_usage_data_.threads_per_workgroup   = std::atoi(tokens[rgResourceUsageCsvFileColumns::kThreadsPerWorkgroup].c_str());
        resource_usage_data_.wavefront_size         = std::atoi(tokens[rgResourceUsageCsvFileColumns::kWavefrontSize].c_str());
        resource_usage_data_.available_lds_bytes     = std::atoi(tokens[rgResourceUsageCsvFileColumns::kAvailableLdsBytes].c_str());
        resource_usage_data_.used_lds_bytes          = std::atoi(tokens[rgResourceUsageCsvFileColumns::kUsedLdsBytes].c_str());
        resource_usage_data_.available_sgprs        = std::atoi(tokens[rgResourceUsageCsvFileColumns::kAvailableSgprs].c_str());
        resource_usage_data_.used_sgprs             = std::atoi(tokens[rgResourceUsageCsvFileColumns::kUsedSgprs].c_str());
        resource_usage_data_.sgpr_spills            = std::atoi(tokens[rgResourceUsageCsvFileColumns::kSgprSpills].c_str());
        resource_usage_data_.available_vgprs        = std::atoi(tokens[rgResourceUsageCsvFileColumns::kAvailableVgprs].c_str());
        resource_usage_data_.used_vgprs             = std::atoi(tokens[rgResourceUsageCsvFileColumns::kUsedVgprs].c_str());
        resource_usage_data_.vgpr_spills            = std::atoi(tokens[rgResourceUsageCsvFileColumns::kVgprSpills].c_str());
        resource_usage_data_.cl_workgroup_x_dimension = std::atoi(tokens[rgResourceUsageCsvFileColumns::kClWorkgroupXDimension].c_str());
        resource_usage_data_.cl_workgroup_y_dimension = std::atoi(tokens[rgResourceUsageCsvFileColumns::kClWorkgroupYDimension].c_str());
        resource_usage_data_.cl_workgroup_z_dimension = std::atoi(tokens[rgResourceUsageCsvFileColumns::kClWorkgroupZDimension].c_str());
        resource_usage_data_.isa_size               = std::atoi(tokens[rgResourceUsageCsvFileColumns::kIsaSize].c_str());
        ret = true;
    }

    return ret;
}
