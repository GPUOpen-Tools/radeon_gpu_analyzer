#pragma once

// C++.
#include <string>

// Local.
#include <Utils/Vulkan/Include/rgPipelineTypes.h>

class RgPsoSerializerVulkan
{
public:
    // Read Pipeline State Object recipe structure from a file.
    static bool ReadStructureFromFile(const std::string& file_path, rgPsoGraphicsVulkan** create_info_array, std::string& error_string);
    static bool ReadStructureFromFile(const std::string& file_path, rgPsoComputeVulkan** create_info_array, std::string& error_string);

    // Write Pipeline State Object recipe structure to a file.
    static bool WriteStructureToFile(rgPsoGraphicsVulkan* create_info, const std::string& file_path, std::string& error_string);
    static bool WriteStructureToFile(rgPsoComputeVulkan* create_info, const std::string& file_path, std::string& error_string);

private:
    RgPsoSerializerVulkan() = delete;
    ~RgPsoSerializerVulkan() = delete;
};