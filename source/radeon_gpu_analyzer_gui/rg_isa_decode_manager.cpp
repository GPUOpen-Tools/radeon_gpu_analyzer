//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the isa decoder manager.
//=============================================================================

// C++.
#include <filesystem>
#include <string>

// Qt.
#include <QApplication>

// Local.
#include "radeon_gpu_analyzer_gui/rg_isa_decode_manager.h"

namespace
{
    // The individual isa spec names.
    const std::unordered_map<amdisa::GpuArchitecture, std::string> kIsaSpecNameMap = {{amdisa::GpuArchitecture::kRdna1, "amdgpu_isa_rdna1.xml"},
                                                                                      {amdisa::GpuArchitecture::kRdna2, "amdgpu_isa_rdna2.xml"},
                                                                                      {amdisa::GpuArchitecture::kRdna3, "amdgpu_isa_rdna3.xml"},
                                                                                      {amdisa::GpuArchitecture::kRdna3_5, "amdgpu_isa_rdna3_5.xml"},
                                                                                      {amdisa::GpuArchitecture::kRdna4, "amdgpu_isa_rdna4.xml"},
                                                                                      {amdisa::GpuArchitecture::kCdna1, "amdgpu_isa_mi100.xml"},
                                                                                      {amdisa::GpuArchitecture::kCdna2, "amdgpu_isa_mi200.xml"},
                                                                                      {amdisa::GpuArchitecture::kCdna3, "amdgpu_isa_mi300.xml"}};
}  // namespace

amdisa::DecodeManager* RgIsaDecodeManager::Get()
{
    if (!decode_manager_)
    {
        const std::string     application_dir = qApp->applicationDirPath().toStdString();
        std::filesystem::path isa_spec_dir_path(application_dir);
        isa_spec_dir_path /= "utils";
        isa_spec_dir_path /= "isa_spec";
        isa_spec_dir_path.make_preferred();

        if (std::filesystem::exists(isa_spec_dir_path) && std::filesystem::is_directory(isa_spec_dir_path))
        {
            std::string              initialize_error_message;
            std::vector<std::string> xml_file_paths;

            for (const auto& isa_spec : kIsaSpecNameMap)
            {
                const auto            isa_spec_name = isa_spec.second;
                std::filesystem::path isa_spec_path(isa_spec_dir_path);
                isa_spec_path /= isa_spec_name;
                isa_spec_path.make_preferred();

                xml_file_paths.emplace_back(isa_spec_path.string());
            }

            decode_manager_ = std::make_unique<amdisa::DecodeManager>();
            decode_manager_->Initialize(xml_file_paths, initialize_error_message);
        }
    }
    return decode_manager_.get();
}
