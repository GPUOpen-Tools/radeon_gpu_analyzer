#pragma once

// C++.
#include <string>

// Local.
#include "source/common/vulkan/rg_pipeline_types.h"

class RgPsoSerializerVulkan
{
public:
    // Read Pipeline State Object recipe structure from a file.
    static bool ReadStructureFromFile(const std::string& file_path, RgPsoGraphicsVulkan** create_info_array, std::string& error_string);
    static bool ReadStructureFromFile(const std::string& file_path, RgPsoComputeVulkan** create_info_array, std::string& error_string);

    // Write Pipeline State Object recipe structure to a file.
    static bool WriteStructureToFile(RgPsoGraphicsVulkan* create_info, const std::string& file_path, std::string& error_string);
    static bool WriteStructureToFile(RgPsoComputeVulkan* create_info, const std::string& file_path, std::string& error_string);

private:
    RgPsoSerializerVulkan() = delete;
    ~RgPsoSerializerVulkan() = delete;
};
