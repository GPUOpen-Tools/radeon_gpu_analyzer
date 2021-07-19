// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_graphics.h"

RgIsaDisassemblyViewGraphics::RgIsaDisassemblyViewGraphics(QWidget* parent)
    : RgIsaDisassemblyView(parent)
{
}

bool RgIsaDisassemblyViewGraphics::PopulateBuildOutput(const std::shared_ptr<RgProjectClone> project_clone, const RgBuildOutputsMap& build_outputs)
{
    bool ret = false;

    std::shared_ptr<RgGraphicsProjectClone> pipeline_clone = std::dynamic_pointer_cast<RgGraphicsProjectClone>(project_clone);
    assert(pipeline_clone != nullptr);
    if (pipeline_clone != nullptr)
    {
        const ShaderInputFileArray& shader_stage_array = pipeline_clone->pipeline.shader_stages;

        // Build artifacts may contain disassembly for shader files that are no longer associated
        // with any stage in the pipeline. Provide the map of the pipeline's current stages, along
        // with the build artifacts that can be loaded.
        ret = PopulateDisassemblyView(shader_stage_array, build_outputs);
    }

    return ret;
}

bool RgIsaDisassemblyViewGraphics::PopulateDisassemblyView(const ShaderInputFileArray& shader_stage_array, const RgBuildOutputsMap& build_output)
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
            std::shared_ptr<RgCliBuildOutputPipeline> gpu_build_output_pipeline =
                std::dynamic_pointer_cast<RgCliBuildOutputPipeline>(gpu_build_output);

            // Step through the outputs map, and load the disassembly data for each input file.
            for (auto input_file_outputs_iter = gpu_build_output_pipeline->per_file_output.begin(); input_file_outputs_iter != gpu_build_output_pipeline->per_file_output.end(); ++input_file_outputs_iter)
            {
                const std::string& input_file_path = input_file_outputs_iter->first;

                // Only load build outputs for files in the given list of source files.
                auto input_file_path_iter = std::find(shader_stage_array.begin(), shader_stage_array.end(), input_file_path);
                if (input_file_path_iter != shader_stage_array.end())
                {
                    // Get the list of outputs for the input file.
                    RgFileOutputs& file_outputs = input_file_outputs_iter->second;
                    const std::vector<RgEntryOutput>& file_entry_outputs = file_outputs.outputs;

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
