// C++.
#include <cassert>

// Qt.
#include <QStyle>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_binary.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

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

        // Build artifacts may contain disassembly for source files that are no longer
        // in the project, so provide a list of files to load, along with the current build output.
        ret = PopulateDisassemblyView(project_clone->build_settings->binary_file_name, build_outputs);
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

bool RgIsaDisassemblyViewBinary::PopulateDisassemblyView(const std::string& binary_file_name, const RgBuildOutputsMap& build_output)
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
            // Step through the outputs map, and load the disassembly data for each input file.
            for (auto output_iter = gpu_build_output->per_file_output.begin(); output_iter != gpu_build_output->per_file_output.end();
                 ++output_iter)
            {
                const std::string& source_file_path = output_iter->first;

                // Only load build outputs for files in the given list of source files.
                if (!binary_file_name.empty() && binary_file_name.compare(source_file_path) == 0)
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
