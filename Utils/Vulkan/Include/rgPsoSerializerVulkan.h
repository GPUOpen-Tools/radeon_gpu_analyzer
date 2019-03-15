#pragma once

// C++.
#include <string>

// Local.
#include <Utils/Vulkan/Include/rgPipelineTypes.h>

class rgPsoSerializerVulkan
{
public:
    // Read Pipeline State Object recipe structure from a file.
    static bool ReadStructureFromFile(const std::string& filePath, rgPsoGraphicsVulkan** ppCreateInfo, std::string& errorString);
    static bool ReadStructureFromFile(const std::string& filePath, rgPsoComputeVulkan** ppCreateInfo, std::string& errorString);

    // Write Pipeline State Object recipe structure to a file.
    static bool WriteStructureToFile(rgPsoGraphicsVulkan* pCreateInfo, const std::string& filePath, std::string& errorString);
    static bool WriteStructureToFile(rgPsoComputeVulkan* pCreateInfo, const std::string& filePath, std::string& errorString);

private:
    rgPsoSerializerVulkan() = delete;
    ~rgPsoSerializerVulkan() = delete;
};