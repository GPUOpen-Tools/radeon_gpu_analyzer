//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for shader ISA Disassembly view for Binary Analysis.
//=============================================================================

// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QStyle>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_binary.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgIsaDisassemblyViewBinary::RgIsaDisassemblyViewBinary(QWidget* parent)
    : RgIsaDisassemblyView(parent)
{
}

bool RgIsaDisassemblyViewBinary::PopulateBuildOutput(const std::shared_ptr<RgProjectClone> project_clone, const RgBuildOutputsMap& build_outputs)
{
    bool ret = false;

    assert(project_clone != nullptr);
    if (project_clone != nullptr && project_clone->build_settings != nullptr)
    {
        std::vector<std::string> file_list;

        for (std::string file : project_clone->build_settings->binary_file_names)
        {
            file_list.push_back(file);
        }

        ui_.targetGpuPushButton->setVisible(false);
        ui_.binary_target_gpu_->setVisible(true);

        // Build artifacts may contain disassembly for source files that are no longer
        // in the project, so provide a list of files to load, along with the current build output.
        ret = PopulateDisassemblyView(file_list, build_outputs);
    }

    return ret;
}

void RgIsaDisassemblyViewBinary::SetBorderStylesheet(bool is_selected)
{
    // Set "selected" property to be utilized by this widget's stylesheet.
    ui_.frame->setProperty(kStrDisassemblyFrameSelected, is_selected);

    // Repolish the widget to ensure the style gets updated.
    ui_.frame->style()->unpolish(ui_.frame);
    ui_.frame->style()->polish(ui_.frame);
}

bool RgIsaDisassemblyViewBinary::PopulateDisassemblyView(const std::vector<std::string>& binary_file_names, const RgBuildOutputsMap& build_output)
{
    bool is_problem_found = false;

    // Iterate through each target GPU's output.
    for (auto gpu_outputs_iter = build_output.begin(); gpu_outputs_iter != build_output.end(); ++gpu_outputs_iter)
    {
        const std::string&                target_gpu       = gpu_outputs_iter->first;
        std::shared_ptr<RgCliBuildOutput> gpu_build_output = gpu_outputs_iter->second;
        bool                              is_valid_output  = gpu_build_output != nullptr;
        assert(is_valid_output);
        if (is_valid_output)
        {
            // Step through the outputs map, and load the disassembly data for each input file.
            for (auto output_iter = gpu_build_output->per_file_output.begin(); output_iter != gpu_build_output->per_file_output.end(); ++output_iter)
            {
                const std::string& source_file_path = output_iter->first;

                bool file_name_found = false;

                for (auto binary_name : binary_file_names)
                {
                    if (binary_name == source_file_path)
                    {
                        file_name_found = true;
                    }
                }

                // Only load build outputs for files in the given list of source files.
                if (file_name_found)
                {
                    // Get the list of outputs for the input file.
                    RgFileOutputs& file_outputs = output_iter->second;

                    // Transform the disassembled entries into a map of GPU -> Entries.
                    // This will make populating the UI much simpler.
                    std::map<std::string, std::vector<RgEntryOutput>> gpu_to_disassembly_csv_entries;
                    std::map<std::string, std::vector<RgEntryOutput>> gpu_to_resource_usage_csv_entries;
                    for (const RgEntryOutput& entry : file_outputs.outputs)
                    {
                        for (const RgOutputItem& outputs : entry.outputs)
                        {
                            if (outputs.file_type == kIsaDisassemblyCsv)
                            {
                                std::vector<RgEntryOutput>& disassembly_csv_file_paths = gpu_to_disassembly_csv_entries[target_gpu];
                                disassembly_csv_file_paths.push_back(entry);
                            }
                            else if (outputs.file_type == kHwResourceUsageFile)
                            {
                                std::vector<RgEntryOutput>& entry_csv_file_paths = gpu_to_resource_usage_csv_entries[target_gpu];
                                entry_csv_file_paths.push_back(entry);
                            }
                        }
                    }

                    // Load the disassembly CSV entries from the build output.
                    bool is_disassembly_data_loaded = PopulateDisassemblyEntries(gpu_to_disassembly_csv_entries);
                    assert(is_disassembly_data_loaded);
                    if (!is_disassembly_data_loaded)
                    {
                        is_problem_found = true;
                    }

                    if (!is_problem_found)
                    {
                        // Load the resource usage CSV entries from the build output.
                        bool is_resource_usage_data_loaded = PopulateResourceUsageEntries(gpu_to_resource_usage_csv_entries);
                        assert(is_resource_usage_data_loaded);
                        if (!is_resource_usage_data_loaded)
                        {
                            is_problem_found = true;
                        }
                    }
                }
            }
        }
    }

    // If the disassembly results loaded correctly, add the target GPU to the dropdown.
    if (!is_problem_found)
    {
        // Populate the target GPU dropdown list with the targets from the build output.
        PopulateTargetGpuList(build_output);
    }

    return !is_problem_found;
}

void RgIsaDisassemblyViewBinary::SetTargetGpuLabel(std::string new_file_path, std::shared_ptr<RgBuildSettings> build_settings)
{
    // Set the target gpu name label.
    std::string target_gpu = "";

    for (size_t file_index = 0; file_index < build_settings->target_gpus.size() && file_index < build_settings->binary_file_names.size(); file_index++)
    {
        if (build_settings->binary_file_names[file_index] == new_file_path)
        {
            target_gpu = build_settings->target_gpus[file_index];
            break;
        }
    }

    if (!target_gpu.empty())
    {
        // Get a mapping of the compute capability to architecture.
        std::map<std::string, std::string> compute_capability_to_arch;
        bool                               has_arch_mapping = RgUtils::GetComputeCapabilityToArchMapping(compute_capability_to_arch);

        // Construct the presented name.
        std::stringstream presented_name;

        // If applicable, prepend the gfx notation (for example, "gfx802/Tonga" for "Tonga").
        std::string gfx_notation;
        bool        has_gfx_notation = RgUtils::GetGfxNotation(target_gpu, gfx_notation);
        if (has_gfx_notation && !gfx_notation.empty())
        {
            presented_name << gfx_notation << "/";
        }
        presented_name << target_gpu;

        // If we have a mapping, let's construct a name that includes
        // the GPU architecture as well: <compute capability> (<architecture>).
        if (has_arch_mapping)
        {
            auto iter = compute_capability_to_arch.find(target_gpu);
            if (iter != compute_capability_to_arch.end())
            {
                presented_name << " (" << iter->second << ")";
            }
        }

        ui_.binary_target_gpu_->setText(QString::fromStdString(presented_name.str()));

        // Also switch the architecture for the model.
        rg_isa_item_model_->SetArchitecture(target_gpu);
    }
}