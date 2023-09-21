// C++.
#include <sstream>
#include <cassert>

// Infra.
#include <external/amdt_base_tools/Include/gtString.h>
#include <external/amdt_os_wrappers/Include/osProcess.h>

// Local.
#include "radeon_gpu_analyzer_gui/rg_cli_launcher.h"
#include "radeon_gpu_analyzer_gui/rg_cli_utils.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_xml_session_config.h"

// OpenCL includes.
#include "radeon_gpu_analyzer_gui/rg_cli_kernel_list_parser.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"

// Vulkan includes.
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_utils_vulkan.h"

// Common between the CLI and the GUI.
#include "source/common/rg_log.h"
#include "source/common/rga_cli_defs.h"

void BuildRgaExecutableCommandString(std::stringstream& command_stream)
{
    // Add the RGA executable name to invoke, and a space.
    command_stream << kStrExecutableName;
    command_stream << " ";
}

// A helper function responsible for building a base command string for CLI invocation.
void BuildCompileProjectCommandString(std::stringstream& command_stream, const std::string& output_path, const std::string& binary_name)
{
    BuildRgaExecutableCommandString(command_stream);

    // Add the current build mode to the command.
    const std::string& current_mode = RgConfigManager::Instance().GetCurrentModeString();
    command_stream << kStrCliOptInputType << " " << current_mode << " ";

    // ISA disassembly in text and CSV formats.
    command_stream << kStrCliOptIsa << " \"" << output_path << "disassem.txt\" " << kStrCliOptParseIsa << " ";

    // Livereg analysis.
    command_stream << kStrCliOptLivereg << " \"" << output_path << "livereg.txt\" ";

    // Include line numbers in the CSV file.
    command_stream << kStrCliOptLineNumbers << " ";

    // Add the output path for the resource usage analysis file.
    command_stream << kStrCliOptStatistics << " \"" << output_path << kStrResourceUsageCsvFilename << "\" ";

    // Output binary file.
    const RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
    command_stream << kStrCliOptBinary << " \"" << output_path << binary_name << "\" ";
    if (current_api == RgProjectAPI::kVulkan)
    {
        if (binary_name.compare(kStrBuildSettingsOutputBinaryFileName) != 0)
        {
            // If the user gave a custom name, instruct the CLI not to add a suffix,
            // so that we use the user's custom file name as is.
            command_stream << kStrCliNoBinaryFileExtensionSwitch << " ";
        }
    }

    // CLI log file path.
    const std::string& cli_log_file_path = RgConfigManager::Instance().GetCLILogFilePath();
    if (!cli_log_file_path.empty())
    {
        command_stream << kStrCliOptLog << " \"" << cli_log_file_path << "\" ";
    }
}

void BuildOutputViewCommandHeader(std::shared_ptr<RgProject> project, const std::string& target_gpu, std::string& invocation_text)
{
    std::stringstream text_stream;

    // Build an output line for each CLI invocation. Print the command string that's about to be invoked.
    text_stream << kStrOutputWindowBuildingProjectHeaderA;

    // Include the project's API in the build header line.
    std::string project_api_string;
    bool is_ok = RgUtils::ProjectAPIToString(project->api, project_api_string);
    assert(is_ok);
    if (is_ok)
    {
        text_stream << project_api_string;
    }

    // Also include the project name and target GPU.
    text_stream << kStrOutputWindowBuildingProjectHeaderB;
    text_stream << project->project_name;
    text_stream << kStrOutputWindowBuildingProjectHeaderC;
    text_stream << target_gpu;

    // Insert a separator line above and below the build output line.
    std::string build_header_string = text_stream.str();
    int num_dashes_to_insert = static_cast<int>(build_header_string.length());
    std::string dashed_lines = std::string(num_dashes_to_insert, '-');

    // Surround the project build header text with dashed line separators.
    std::stringstream cmd_line_output_stream;
    cmd_line_output_stream << dashed_lines << std::endl;
    cmd_line_output_stream << build_header_string << std::endl;
    cmd_line_output_stream << dashed_lines << std::endl;

    invocation_text = cmd_line_output_stream.str();
}

bool RgCliLauncher::BuildProjectCloneOpencl(std::shared_ptr<RgProject> project, int clone_index, const std::string& output_path, const std::string& binary_name,
    std::function<void(const std::string&)> cli_output_handling_callback, std::vector<std::string>& gpu_built, bool& cancel_signal)
{
    bool ret = false;
    if (project != nullptr)
    {
        RgLog::file << kStrLogBuildingProjectClone1 << project->project_name << kStrLogBuildingProjectClone2 << clone_index << std::endl;

        // A stream of all text collected from CLI invocation.
        std::stringstream full_cli_output;

        // Verify that the clone index is valid for the given project.
        int num_clones = static_cast<int>(project->clones.size());
        bool is_clone_index_valid = (clone_index >= 0 && clone_index < num_clones);
        assert(is_clone_index_valid);
        if (is_clone_index_valid)
        {
            std::shared_ptr<RgProjectClone> target_clone = project->clones[clone_index];

            // Generate the command line invocation command.
            std::stringstream cmd;

            // Build the compile project command string.
            BuildCompileProjectCommandString(cmd, output_path, binary_name);

            // Build settings.
            std::string build_settings;
            std::shared_ptr<RgBuildSettingsOpencl> cl_build_settings =
                std::static_pointer_cast<RgBuildSettingsOpencl>(target_clone->build_settings);
            assert(cl_build_settings != nullptr);
            ret = RgCliUtils::GenerateOpenclBuildSettingsString(*cl_build_settings, build_settings);
            assert(ret);
            if (!build_settings.empty())
            {
                cmd << build_settings << " ";
            }

            // Execute the CLI for each target GPU, appending the GPU to the command.
            for (const std::string& target_gpu : target_clone->build_settings->target_gpus)
            {
                if (!cancel_signal)
                {
                    // Print the command string that's about to be used to invoke the RGA CLI build process.
                    std::string cli_invocation_command_string;
                    BuildOutputViewCommandHeader(project, target_gpu, cli_invocation_command_string);

                    // Append the command string header text to the output string.
                    std::stringstream cmd_line_output_stream;
                    cmd_line_output_stream << cli_invocation_command_string;

                    // Construct the full CLI command string including the current target GPU.
                    std::stringstream full_cmd_with_gpu;
                    full_cmd_with_gpu << cmd.str();

                    // Specify the Metadata file path.
                    full_cmd_with_gpu << kStrCliOptSessionMetadata << " \"" << output_path << target_gpu << "_" << kStrSessionMetadataFilename << "\" ";

                    // Specify which GPU to build outputs for.
                    full_cmd_with_gpu << kStrCliOptAsic << " " << target_gpu << " ";

                    // Append each input file to the end of the CLI command.
                    for (const RgSourceFileInfo& file_info : target_clone->source_files)
                    {
                        // Surround the path to the input file with quotes to prevent breaking the CLI parser.
                        full_cmd_with_gpu << "\"";
                        full_cmd_with_gpu << file_info.file_path;
                        full_cmd_with_gpu << "\" ";
                    }

                    // Add the full CLI execution string to the output window's log.
                    cmd_line_output_stream << full_cmd_with_gpu.str();

                    // Send the new output text to the output window.
                    if (cli_output_handling_callback != nullptr)
                    {
                        cli_output_handling_callback(cmd_line_output_stream.str());
                    }

                    // Execute the command and grab the output.
                    RgLog::file << kStrLogLaunchingCli << RgLog::noflush << std::endl << full_cmd_with_gpu.str() << std::endl << RgLog::flush;
                    gtString cmd_line_output_as_gt_str;
                    ret = osExecAndGrabOutput(full_cmd_with_gpu.str().c_str(), cancel_signal, cmd_line_output_as_gt_str);

                    assert(ret);
                    if (ret)
                    {
                        // Add the GPU to the output list if it was built successfully.
                        gpu_built.push_back(target_gpu);
                    }

                    // Append the CLI's output to the string containing the entire execution output.
                    full_cli_output << cmd_line_output_as_gt_str.asASCIICharArray();

                    // Invoke the callback used to send new CLI output to the GUI.
                    if (cli_output_handling_callback != nullptr)
                    {
                        cli_output_handling_callback(cmd_line_output_as_gt_str.asASCIICharArray());
                    }
                }
                else
                {
                    // Stop the process if build is canceled.
                    break;
                }
            }
        }
        else
        {
            // Invoke the callback used to send new CLI output to the GUI.
            if (cli_output_handling_callback != nullptr)
            {
                std::stringstream error_stream;
                error_stream << kStrOutputWindowBuildingProjectFailedText;
                error_stream << kStrOutputWindowBuildingProjectFailedInvalidClone;
                cli_output_handling_callback(error_stream.str());
            }
        }
    }

    return ret;
}

bool RgCliLauncher::BuildProjectCloneVulkan(std::shared_ptr<RgProject> project, int clone_index, const std::string& output_path, const std::string& binary_name,
    std::function<void(const std::string&)> cli_output_handling_callback, std::vector<std::string>& gpus_built, bool& cancel_signal)
{
    Q_UNUSED(binary_name);

    bool ret = false;
    if (project != nullptr)
    {
        RgLog::file << kStrLogBuildingProjectClone1 << project->project_name << kStrLogBuildingProjectClone2 << clone_index << std::endl;

        // A stream of all text collected from CLI invocation.
        std::stringstream full_cli_output;

        // Verify that the clone index is valid for the given project.
        int num_clones = static_cast<int>(project->clones.size());
        bool is_clone_index_valid = (clone_index >= 0 && clone_index < num_clones);
        assert(is_clone_index_valid);
        if (is_clone_index_valid)
        {
            std::shared_ptr<RgProjectClone> target_clone = project->clones[clone_index];
            assert(target_clone != nullptr);
            if (target_clone != nullptr)
            {
                std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(target_clone);
                assert(vulkan_clone != nullptr);
                if (vulkan_clone != nullptr)
                {
                    // Generate the command line invocation command.
                    std::stringstream cmd;

                    // Build settings.
                    std::string build_settings;
                    std::shared_ptr<RgBuildSettingsVulkan> build_settings_vulkan =
                        std::static_pointer_cast<RgBuildSettingsVulkan>(target_clone->build_settings);
                    assert(build_settings_vulkan != nullptr);

                    // Build the compile project command string.
                    BuildCompileProjectCommandString(cmd, output_path, build_settings_vulkan->binary_file_name);

                    ret = RgCliUtils::GenerateVulkanBuildSettingsString(*build_settings_vulkan, build_settings);
                    assert(ret);
                    if (!build_settings.empty())
                    {
                        cmd << build_settings << " ";
                    }

                    std::shared_ptr<RgUtilsVulkan> vulkan_util = std::dynamic_pointer_cast<RgUtilsVulkan>(RgUtilsGraphics::CreateUtility(RgProjectAPI::kVulkan));
                    assert(vulkan_util != nullptr);
                    if (vulkan_util != nullptr)
                    {
                        // Execute the CLI for each target GPU, appending the GPU to the command.
                        for (const std::string& target_gpu : target_clone->build_settings->target_gpus)
                        {
                            if (!cancel_signal)
                            {
                                // Print the command string that's about to be used to invoke the RGA CLI build process.
                                std::string cli_invocation_command_string;
                                BuildOutputViewCommandHeader(project, target_gpu, cli_invocation_command_string);

                                // Append the command string header text to the output string.
                                std::stringstream cmd_line_output_stream;
                                cmd_line_output_stream << cli_invocation_command_string;

                                // Construct the full CLI command string including the current target GPU.
                                std::stringstream full_cmd_with_gpu;
                                full_cmd_with_gpu << cmd.str();

                                // Specify the Metadata file path.
                                full_cmd_with_gpu << kStrCliOptSessionMetadata << " \"" << output_path << target_gpu << "_" << kStrSessionMetadataFilename << "\" ";

                                // Specify which GPU to build outputs for.
                                full_cmd_with_gpu << kStrCliOptAsic << " " << target_gpu << " ";

                                // Provide the pipeline state object configuration file.
                                for (auto pso_state_file : vulkan_clone->pso_states)
                                {
                                    // Only append the path for the active PSO config.
                                    if (pso_state_file.is_active)
                                    {
                                        // Append the pipeline's state file path.
                                        full_cmd_with_gpu << kStrCliOptPso << " \"" << pso_state_file.pipeline_state_file_path << "\" ";
                                        break;
                                    }
                                }

                                // Append each active pipeline stage's input file to the command line.
                                if (vulkan_clone->pipeline.type == RgPipelineType::kGraphics)
                                {
                                    size_t first_stage = static_cast<size_t>(RgPipelineStage::kVertex);
                                    size_t last_stage = static_cast<size_t>(RgPipelineStage::kFragment);

                                    // Step through each stage in a graphics pipeline. If the project's
                                    // stage is not empty, append the stage's shader file to the cmdline.
                                    for (size_t stage_index = first_stage; stage_index <= last_stage; ++stage_index)
                                    {
                                        RgPipelineStage current_stage = static_cast<RgPipelineStage>(stage_index);

                                        // Try to find the given stage within the pipeline's stage map.
                                        const auto& stage_input_file = vulkan_clone->pipeline.shader_stages[stage_index];
                                        if (!stage_input_file.empty())
                                        {
                                            // A source file exists in this stage. Append it to the command line.
                                            std::string stage_abbreviation = vulkan_util->PipelineStageToAbbreviation(current_stage);

                                            // Check to see if the file has txt extension.
                                            if (RgUtils::IsSpvasTextFile(stage_input_file, stage_abbreviation))
                                            {
                                                // Append "-spvas" to stage abbreviation.
                                                stage_abbreviation += kStrCliOptSpvasTextFile;
                                            }

                                            // Append the stage type and shader file path to the command line.
                                            full_cmd_with_gpu << "--" << stage_abbreviation << " \"" << stage_input_file << "\" ";
                                        }
                                    }
                                }
                                else if (vulkan_clone->pipeline.type == RgPipelineType::kCompute)
                                {
                                    RgPipelineStage current_stage = RgPipelineStage::kCompute;

                                    // Does the pipeline have a compute shader source file?
                                    const auto& compute_shader_input_source_file_path = vulkan_clone->pipeline.shader_stages[static_cast<size_t>(current_stage)];
                                    if (!compute_shader_input_source_file_path.empty())
                                    {
                                        // A source file exists in this stage. Append it to the command line.
                                        std::string stage_abbreviation = vulkan_util->PipelineStageToAbbreviation(current_stage);

                                        // Check to see if the file has txt extension.
                                        if (RgUtils::IsSpvasTextFile(compute_shader_input_source_file_path, stage_abbreviation))
                                        {
                                            // Append "-spvas" to stage abbreviation.
                                            stage_abbreviation += kStrCliOptSpvasTextFile;
                                        }

                                        // Append the stage type and shader file path to the command line.
                                        full_cmd_with_gpu << "--" << stage_abbreviation << " \"" << compute_shader_input_source_file_path << "\" ";
                                    }
                                }
                                else
                                {
                                    // The pipeline type can only be graphics or compute.
                                    // If we get here something is wrong with the project clone.
                                    assert(false);
                                    ret = false;
                                }

                                // Verify that all operations up to this point were successful.
                                assert(ret);
                                if (ret)
                                {
                                    // Add the full CLI execution string to the output window's log.
                                    cmd_line_output_stream << full_cmd_with_gpu.str();

                                    // Send the new output text to the output window.
                                    if (cli_output_handling_callback != nullptr)
                                    {
                                        cli_output_handling_callback(cmd_line_output_stream.str());
                                    }

                                    // Execute the command and grab the output.
                                    RgLog::file << kStrLogLaunchingCli << RgLog::noflush << std::endl << full_cmd_with_gpu.str() << std::endl << RgLog::flush;
                                    gtString cmd_line_output_as_gt_str;
                                    ret = osExecAndGrabOutput(full_cmd_with_gpu.str().c_str(), cancel_signal, cmd_line_output_as_gt_str);

                                    assert(ret);
                                    if (ret)
                                    {
                                        // Add the GPU to the output list if it was built successfully.
                                        gpus_built.push_back(target_gpu);
                                    }

                                    // Append the CLI's output to the string containing the entire execution output.
                                    full_cli_output << cmd_line_output_as_gt_str.asASCIICharArray();

                                    // Invoke the callback used to send new CLI output to the GUI.
                                    if (cli_output_handling_callback != nullptr)
                                    {
                                        cli_output_handling_callback(cmd_line_output_as_gt_str.asASCIICharArray());
                                    }
                                }
                                else
                                {
                                    // Send a failure error message to the output window.
                                    if (cli_output_handling_callback != nullptr)
                                    {
                                        cli_output_handling_callback(kStrErrFailedToGenerateBuildCommand);
                                    }
                                }
                            }
                            else
                            {
                                // Stop the process if build is canceled.
                                break;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // Invoke the callback used to send new CLI output to the GUI.
            if (cli_output_handling_callback != nullptr)
            {
                std::stringstream error_stream;
                error_stream << kStrOutputWindowBuildingProjectFailedText;
                error_stream << kStrOutputWindowBuildingProjectFailedInvalidClone;
                cli_output_handling_callback(error_stream.str());
            }
        }
    }

    return ret;
}

bool RgCliLauncher::DisassembleSpvToText(const std::string& compiler_bin_folder, const std::string& spv_full_file_path,
                                         const std::string& output_file_path, std::string& cli_output)
{
    bool ret = false;

    // Generate the command line backend invocation command.
    std::stringstream command_stream;
    BuildRgaExecutableCommandString(command_stream);

    // Add the current build mode to the command.
    const std::string& current_mode = RgConfigManager::Instance().GetCurrentModeString();
    command_stream << kStrCliOptInputType << " " << current_mode << " ";

    // Append the alternative compiler path if it's not empty.
    if (!compiler_bin_folder.empty())
    {
        command_stream << kStrCliOptCompilerBinDir << " \"" << compiler_bin_folder << "\" ";
    }

    // Append the ISA text output path as well as the target SPIR-V to disassemble.
    command_stream << kStrCliOptVulkanSpvDis << " \"" << output_file_path << "\" \"" << spv_full_file_path << "\"";

    // Launch the command line backend to generate the version info file.
    bool cancel_signal = false;
    gtString cli_output_as_gt_str;
    bool is_launch_successful = osExecAndGrabOutput(command_stream.str().c_str(), cancel_signal, cli_output_as_gt_str);
    assert(is_launch_successful);

    if (is_launch_successful)
    {
        cli_output = cli_output_as_gt_str.asASCIICharArray();

        // Verify that the file was indeed created.
        bool is_output_file_created = RgUtils::IsFileExists(output_file_path);
        assert(is_output_file_created);
        ret = is_output_file_created;
    }

    return ret;
}

bool RgCliLauncher::GenerateVersionInfoFile(const std::string& full_path)
{
    bool ret = false;

    // Generate the command line backend invocation command.
    std::stringstream cmd;

    // Add the executable name.
    BuildRgaExecutableCommandString(cmd);

    // Add the version-info option to the command.
    cmd << kStrCliOptVersionInfo << " \"" << full_path << "\"";

    // Launch the command line backend to generate the version info file.
    bool cancel_signal = false;
    gtString cli_output_as_gt_str;
    bool is_launch_successful = osExecAndGrabOutput(cmd.str().c_str(), cancel_signal, cli_output_as_gt_str);
    assert(is_launch_successful);

    if (is_launch_successful)
    {
        // Verify that the file was indeed created.
        bool is_output_file_created = RgUtils::IsFileExists(full_path);
        assert(is_output_file_created);
        ret = is_output_file_created;
    }

    return ret;
}

bool RgCliLauncher::ListKernels(std::shared_ptr<RgProject> project, int clone_index, std::map<std::string, EntryToSourceLineRange>& entrypoint_line_numbers)
{
    bool is_parsing_failed = false;

    assert(project != nullptr);
    if (project != nullptr)
    {
        bool is_valid_index = clone_index >= 0 && clone_index < project->clones.size();
        assert(is_valid_index);
        if (is_valid_index && !project->clones.empty() && project->clones[clone_index] != nullptr)
        {
            // Append each input file to the end of the CLI command.
            for (const RgSourceFileInfo& file_info : project->clones[clone_index]->source_files)
            {
                // Generate the command line backend invocation command.
                std::stringstream cmd;

                // Add the executable name.
                BuildRgaExecutableCommandString(cmd);

                // Add the current build mode to the command.
                const std::string& current_mode = RgConfigManager::Instance().GetCurrentModeString();
                cmd << kStrCliOptInputType << " " << current_mode << " ";

                std::stringstream full_cmd_with_gpu;
                full_cmd_with_gpu << cmd.str();

                const std::string& source_file_path = file_info.file_path;

                // Add the version-info option to the command.
                cmd << kStrCliOptListKernels << " " << "\"" << source_file_path << "\"";

                // Launch the command line backend to generate the version info file.
                bool cancel_signal = false;
                gtString cli_output_as_gt_str;
                bool is_launch_successful = osExecAndGrabOutput(cmd.str().c_str(), cancel_signal, cli_output_as_gt_str);
                assert(is_launch_successful);
                if (is_launch_successful)
                {
                    // Parse the output and dump it into entrypointLineNumbers.
                    is_parsing_failed = !RgCliKernelListParser::ReadListKernelsOutput(cli_output_as_gt_str.asASCIICharArray(),
                                                                                    entrypoint_line_numbers[source_file_path]);
                }
                else
                {
                    is_parsing_failed = true;
                }
            }
        }
    }

    is_parsing_failed = entrypoint_line_numbers.empty();

    return !is_parsing_failed;
}
