//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for shader ISA Disassembly view for OpenCL.
//=============================================================================

// C++.
#include <cassert>

// Qt.
#include <QStyle>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

RgIsaDisassemblyViewOpencl::RgIsaDisassemblyViewOpencl(QWidget* parent)
    : RgIsaDisassemblyView(parent)
{
}

bool RgIsaDisassemblyViewOpencl::PopulateBuildOutput(const std::shared_ptr<RgProjectClone> project_clone, const RgBuildOutputsMap& build_outputs)
{
    bool ret = false;

    assert(project_clone != nullptr);
    if (project_clone != nullptr)
    {
        std::vector<RgSourceFileInfo>& project_source_files = project_clone->source_files;

        ui_.targetGpuPushButton->setVisible(true);
        ui_.binary_target_gpu_->setVisible(false);

        // Build artifacts may contain disassembly for source files that are no longer
        // in the project, so provide a list of files to load, along with the current build output.
        ret = PopulateDisassemblyView(project_source_files, build_outputs);
    }

    return ret;
}

bool RgIsaDisassemblyViewOpencl::IsLineCorrelationSupported() const
{
    return true;
}

void RgIsaDisassemblyViewOpencl::SetBorderStylesheet(bool is_selected)
{
    // Set "selected" property to be utilized by this widget's stylesheet.
    ui_.frame->setProperty(kStrDisassemblyFrameSelected, is_selected);

    // Repolish the widget to ensure the style gets updated.
    ui_.frame->style()->unpolish(ui_.frame);
    ui_.frame->style()->polish(ui_.frame);
}

bool RgIsaDisassemblyViewOpencl::PopulateDisassemblyView(const std::vector<RgSourceFileInfo>& source_files, const RgBuildOutputsMap& build_output)
{
    bool is_problem_found = false;

    // Iterate through each target GPU's output.
    for (auto gpu_outputs_iter = build_output.begin(); gpu_outputs_iter != build_output.end(); ++gpu_outputs_iter)
    {
        const std::string& target_gpu = gpu_outputs_iter->first;
        std::shared_ptr<RgCliBuildOutput> gpu_build_output = gpu_outputs_iter->second;
        bool is_valid_output = gpu_build_output != nullptr;
        assert(is_valid_output);
        if (is_valid_output)
        {
            std::shared_ptr<RgCliBuildOutputOpencl> gpu_build_output_opencl =
                std::dynamic_pointer_cast<RgCliBuildOutputOpencl>(gpu_build_output);

            // Step through the outputs map, and load the disassembly data for each input file.
            for (auto output_iter = gpu_build_output_opencl->per_file_output.begin(); output_iter != gpu_build_output_opencl->per_file_output.end(); ++output_iter)
            {
                const std::string& source_file_path = output_iter->first;

                // Only load build outputs for files in the given list of source files.
                RgSourceFilePathSearcher source_file_searcher(source_file_path);
                auto source_file_iter = std::find_if(source_files.begin(), source_files.end(), source_file_searcher);
                if (source_file_iter != source_files.end())
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
