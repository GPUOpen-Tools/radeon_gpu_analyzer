// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgResourceUsageCsvFileParser.h>

bool rgResourceUsageCsvFileParser::ProcessLineTokens(const std::vector<std::string>& tokens)
{
    bool ret = false;

    // Verify that the number of tokens matches the number of columns in the file being parsed.
    int numTokens = static_cast<int>(tokens.size());
    assert(numTokens == rgResourceUsageCsvFileColumns::Count);
    if (numTokens == rgResourceUsageCsvFileColumns::Count)
    {
        // Extract all info from the file's line tokens.
        m_resourceUsageData.m_device                = tokens[rgResourceUsageCsvFileColumns::Device];
        m_resourceUsageData.m_scratchMemory         = std::atoi(tokens[rgResourceUsageCsvFileColumns::ScratchMemory].c_str());
        m_resourceUsageData.m_threadsPerWorkgroup   = std::atoi(tokens[rgResourceUsageCsvFileColumns::ThreadsPerWorkgroup].c_str());
        m_resourceUsageData.m_wavefrontSize         = std::atoi(tokens[rgResourceUsageCsvFileColumns::WavefrontSize].c_str());
        m_resourceUsageData.m_availableLdsBytes     = std::atoi(tokens[rgResourceUsageCsvFileColumns::AvailableLdsBytes].c_str());
        m_resourceUsageData.m_usedLdsBytes          = std::atoi(tokens[rgResourceUsageCsvFileColumns::UsedLdsBytes].c_str());
        m_resourceUsageData.m_availableSgprs        = std::atoi(tokens[rgResourceUsageCsvFileColumns::AvailableSgprs].c_str());
        m_resourceUsageData.m_usedSgprs             = std::atoi(tokens[rgResourceUsageCsvFileColumns::UsedSgprs].c_str());
        m_resourceUsageData.m_sgprSpills            = std::atoi(tokens[rgResourceUsageCsvFileColumns::SgprSpills].c_str());
        m_resourceUsageData.m_availableVgprs        = std::atoi(tokens[rgResourceUsageCsvFileColumns::AvailableVgprs].c_str());
        m_resourceUsageData.m_usedVgprs             = std::atoi(tokens[rgResourceUsageCsvFileColumns::UsedVgprs].c_str());
        m_resourceUsageData.m_vgprSpills            = std::atoi(tokens[rgResourceUsageCsvFileColumns::VgprSpills].c_str());
        m_resourceUsageData.m_clWorkgroupXDimension = std::atoi(tokens[rgResourceUsageCsvFileColumns::ClWorkgroupXDimension].c_str());
        m_resourceUsageData.m_clWorkgroupYDimension = std::atoi(tokens[rgResourceUsageCsvFileColumns::ClWorkgroupYDimension].c_str());
        m_resourceUsageData.m_clWorkgroupZDimension = std::atoi(tokens[rgResourceUsageCsvFileColumns::ClWorkgroupZDimension].c_str());
        m_resourceUsageData.m_isaSize               = std::atoi(tokens[rgResourceUsageCsvFileColumns::IsaSize].c_str());
        ret = true;
    }

    return ret;
}