#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_XML_CLI_VERSION_INFO_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_XML_CLI_VERSION_INFO_H_

// C++.
#include <memory>

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"

// A class responsible for parsing the CLI's version-info results.
class RgXMLCliVersionInfo
{
public:
    // Read the version info from file.
    static bool ReadVersionInfo(const std::string& version_info_file_path, std::shared_ptr<RgCliVersionInfo>& version_info);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_XML_CLI_VERSION_INFO_H_
