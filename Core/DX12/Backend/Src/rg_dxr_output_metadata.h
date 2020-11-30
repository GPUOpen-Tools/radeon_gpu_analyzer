#pragma once

// C++.
#include <string>
#include <vector>
#include <memory>

// Local.
#include "rg_dx12_data_types.h"

namespace rga
{
    class RgDxrOutputMetadata
    {
    public:
        RgDxrOutputMetadata() = delete;
        ~RgDxrOutputMetadata() = delete;
        static bool WriteOutputMetadata(const std::string& jsonFileName, const std::vector<RgDxrPipelineResults> pipelineResults, std::string& errorMsg);
        static bool ReadOutputMetadata(const std::string& jsonFileName, std::vector<RgDxrPipelineResults>& pipelineResults, std::string& errorMsg);
    };
};