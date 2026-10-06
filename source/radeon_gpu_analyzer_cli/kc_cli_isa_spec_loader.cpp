//=============================================================================
// Copyright (c) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for ISA spec loader utility.
//=============================================================================

// C++.
#include <unordered_map>

// IsaSpec.
#include "amdisa/isa_decoder.h"

// Common.
#include "common/rga_shared_utils.h"
#include "common/rg_log.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_isa_parser.h"
#include "radeon_gpu_analyzer_backend/be_isa_spec_metadata.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_isa_spec_loader.h"

// The individual isa spec names.
const std::unordered_map<amdisa::GpuArchitecture, std::string> kIsaSpecNameMap = {{amdisa::GpuArchitecture::kRdna1, "amdgpu_isa_rdna1.xml"},
                                                                                  {amdisa::GpuArchitecture::kRdna2, "amdgpu_isa_rdna2.xml"},
                                                                                  {amdisa::GpuArchitecture::kRdna3, "amdgpu_isa_rdna3.xml"},
                                                                                  {amdisa::GpuArchitecture::kRdna3_5, "amdgpu_isa_rdna3_5.xml"},
                                                                                  {amdisa::GpuArchitecture::kRdna4, "amdgpu_isa_rdna4.xml"},
                                                                                  {amdisa::GpuArchitecture::kCdna1, "amdgpu_isa_cdna1.xml"},
                                                                                  {amdisa::GpuArchitecture::kCdna2, "amdgpu_isa_cdna2.xml"},
                                                                                  {amdisa::GpuArchitecture::kCdna3, "amdgpu_isa_cdna3.xml"},
                                                                                  {amdisa::GpuArchitecture::kCdna4, "amdgpu_isa_cdna4.xml"},
                                                                                  {amdisa::GpuArchitecture::kCdna5, "amdgpu_isa_cdna5.xml"}};

bool KcCliIsaSpecLoader::LoadIsaSpecsFromXML(bool is_parsed_isa_required, bool is_isa_spec_metadata_required, const std::vector<std::string>& target_gpus)
{
    bool ret = true;
    if (is_parsed_isa_required)
    {
        std::set<std::string>                          xml_file_paths;
        std::map<amdisa::GpuArchitecture, std::string> xml_file_paths_map;

        // Only load the needed xml spec files.
        for (const auto& target_gpu : target_gpus)
        {
            amdisa::GpuArchitecture architecture = amdisa::GpuArchitecture::kUnknown;
            RgaSharedUtils::GetGpuArchitectureFromTarget(target_gpu, architecture);
            auto it                              = kIsaSpecNameMap.find(architecture);
            if (it != kIsaSpecNameMap.end())
            {
                xml_file_paths.insert(it->second);
                if (is_isa_spec_metadata_required)
                {
                    xml_file_paths_map[architecture] = it->second;
                }
            }
        }

        std::string decoder_error_msg;
        if (!IsaParser::InitializeDecoder(xml_file_paths, decoder_error_msg))
        {
            RgLog::stdErr << decoder_error_msg << std::endl;
            ret = false;
        }

        if (is_isa_spec_metadata_required)
        {
            std::string explorer_error_msg;
            if (!beKA::BeIsaSpecExplorer::InitializeExplorer(xml_file_paths_map, explorer_error_msg))
            {
                RgLog::stdErr << explorer_error_msg << std::endl;
                ret = false;
            }
        }
    }

    return ret;
}
