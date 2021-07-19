//=================================================================
// Copyright 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// XML.
#include "tinyxml2/Include/tinyxml2.h"

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "AMDTBaseTools/Include/gtString.h"
#include "AMDTOSWrappers/Include/osFilePath.h"
#include "AMDTOSWrappers/Include/osFile.h"
#include "AMDTOSWrappers/Include/osDirectory.h"
#include "AMDTOSWrappers/Include/osProcess.h"
#include "AMDTOSWrappers/Include/osEnvironmentVariable.h"
#include "update_check_api.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Backend.
#include "radeon_gpu_analyzer_backend/be_static_isa_analyzer.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander.h"
#include "common/rga_xml_constants.h"
#include "common/rga_shared_utils.h"
#include "common/rg_log.h"
#include "common/rga_version_info.h"

#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <dirent.h>
#endif

using namespace beKA;

// Constants.
static const gtString  kRgaCliLogFileName           = L"rga_cli";
static const gtString  kRgaCliLogFileExt            = L"log";
static const gtString  kRgaCliParsedIsaFileExt      = L"csv";
static const gtString  kRgaCliTempStdoutFilename    = L"rga_stdout";
static const gtString  kRgaCliTempStdoutFileExt     = L"txt";
static const gtString  kRgaCliTempStderrFilename    = L"rga_stderr";
static const gtString  kRgaCliTempStderrFileExt     = L"txt";
static const char*     kStrFopenModeAppend          = "a";
static const char*     STR_TARGET_DETECTED          = "Target GPU detected:";

// Constants: error messages.
static const char* kStrErrorCannotLocateLiveregAnalyzer     = "Error: cannot locate the live register analyzer.";
static const char* kStrErrorCannotLaunchLiveregAnalyzer     = "Error: cannot launch the live register analyzer.";
static const char* kStrErrorCannotLaunchCfgAnalyzer         = "Error: cannot launch the control graph generator.";
static const char* kStrErrorCannotFindIsaFile               = "Error: ISA file not found.";
static const char* kStrErrorCouldNotDetectTarget            = "Error: could not detect target GPU -> ";
static const char* kStrErrorAmbiguousTarget                 = "Error: ambiguous target GPU name -> ";
static const char* kStrErrorFailedToOpenLogFile             = "Error: failed to open log file: ";

// Constants: warning messages.
static const char* kStrWarningFailedToDeleteLogFiles = "Warning: failed to delete old log files.";

// Constants: info messages.
static const char* kStrLaunchingExternalProcess = "Info: launching external process: \n";

// Log messages.
static const char* kStrRgaCliLogsStart = "RGA CLI process started.";

#ifdef WIN32
const int  kWindowsDateStringLength                 = 14;
const int  kWindowsDateStringYearOffset             = 10;
const int  kWindowsDateStringMonthOffset            =  7;
const int  kWindowsDateStringDayOffset              =  4;
#endif

// This container references the disabled devices. At certain times it might be empty.
static const std::vector<std::string>  kRgaDisabledDevices = { };

static bool GetRGATempDir(osDirectory & dir);

static std::string AdjustBaseFileName(const std::string& user_input_filename, const std::string& device, const char* base_filename)
{
    std::string ret = user_input_filename;
    if (ret.empty())
    {
        // Generate a default file name if needed.
        gtString  temp_isa_filename, isa_file_ext;
        temp_isa_filename << (std::string(kStrDefaultFilenameIsa) + device).c_str();
        isa_file_ext << kStrDefaultExtensionIsa;
        gtString isa_output_filename = KcUtils::ConstructTempFileName(temp_isa_filename, isa_file_ext);
        ret = isa_output_filename.asASCIICharArray();
    }
    else
    {
        // If the given path is a directory, generate the file in that directory.
        gtString isa_filename_gtstr;
        isa_filename_gtstr << user_input_filename.c_str();
        osFilePath isa_file_path(isa_filename_gtstr);
        if (isa_file_path.isDirectory())
        {
            gtString default_isa_filename;
            default_isa_filename << base_filename;

            osDirectory dir(isa_file_path);
            isa_file_path.setFileDirectory(dir);
            isa_file_path.setFileName(default_isa_filename);
            ret = isa_file_path.asString().asASCIICharArray();
        }
    }
    return ret;
}

bool KcUtils::ValidateShaderFileName(const char* shader_type, const std::string& shader_filename, std::stringstream& log_msg)
{
    bool is_shader_name_valid = true;
    gtString shader_filename_as_gtstr;
    shader_filename_as_gtstr << shader_filename.c_str();
    osFilePath shaderFile(shader_filename_as_gtstr);

    if (!shaderFile.exists())
    {
        const char* const kStrErrorCannotFindShaderPrefix = "Error: cannot find ";
        const char* const kStrErrorCannotFindShaderSuffix = " shader: ";
        is_shader_name_valid = false;
        log_msg << kStrErrorCannotFindShaderPrefix << shader_type << kStrErrorCannotFindShaderSuffix << shader_filename << std::endl;
    }

    return is_shader_name_valid;
}

bool KcUtils::ValidateShaderOutputDir(const std::string& output_filename, std::stringstream& log_msg)
{
    bool is_shader_output_dir_valid = true;
    gtString shader_filename_as_gtstr;
    shader_filename_as_gtstr << output_filename.c_str();
    osFilePath shader_file(shader_filename_as_gtstr);
    osDirectory output_dir;
    shader_file.getFileDirectory(output_dir);

    // If the directory is empty then we assume the output directory is the active directory which should exist.
    is_shader_output_dir_valid = output_dir.asString().isEmpty() || output_dir.exists();

    if (!is_shader_output_dir_valid)
    {
        log_msg << kStrErrorCannotFindOutputDir << output_dir.directoryPath().asString().asASCIICharArray() << std::endl;
    }

    return is_shader_output_dir_valid;
}

bool KcUtils::AdjustRenderingPipelineOutputFileNames(const std::string& base_output_filename, const std::string& default_suffix,
                                                     const std::string& default_ext, const std::string& device,
                                                     BeProgramPipeline& pipeline_files)
{
    // Stage abbreviations for output file names.
    static const char* kStrVertexStageNameAbbreviation = "vert";
    static const char* kStrTessellationControlStageNameAbbreviation = "tesc";
    static const char* kStrTessellationEvaluationStageNameAbbreviation = "tese";
    static const char* kStrGeometryStageNameAbbreviation = "geom";
    static const char* kStrFragmentStageNameAbbreviation = "frag";
    static const char* kStrComputeStageNameAbbreviation = "comp";

    // Clear the existing pipeline.
    pipeline_files.ClearAll();

    // Isolate the original file name.
    gtString output_file_as_gtstr;
    output_file_as_gtstr << base_output_filename.c_str();
    osFilePath output_file_path(output_file_as_gtstr);

    if (!base_output_filename.empty() && output_file_path.isDirectory())
    {
        osDirectory output_dir(output_file_as_gtstr);
        output_file_path.setFileDirectory(output_dir);
        output_file_path.clearFileName();
        output_file_path.clearFileExtension();
    }

    osDirectory output_dir;
    bool status = true;

    // Use system temp folder if no path is provided by a user.
    if (base_output_filename.empty())
    {
        status = GetRGATempDir(output_dir);
    }
    else
    {
        output_file_path.getFileDirectory(output_dir);
    }

    if (status)
    {
        // File name.
        gtString original_filename;
        output_file_path.getFileName(original_filename);

        // File extension.
        gtString original_file_extension;
        output_file_path.getFileExtension(original_file_extension);

        // Make the adjustments.
        gtString fixed_filename;
        fixed_filename << output_dir.directoryPath().asString(true) << device.c_str();

        if (!original_filename.isEmpty())
        {
            pipeline_files.vertex_shader << fixed_filename << "_" << original_filename.asASCIICharArray();
            pipeline_files.tessellation_control_shader << fixed_filename << "_" << original_filename.asASCIICharArray();
            pipeline_files.tessellation_evaluation_shader << fixed_filename << "_" << original_filename.asASCIICharArray();
            pipeline_files.geometry_shader << fixed_filename << "_" << original_filename.asASCIICharArray();
            pipeline_files.fragment_shader << fixed_filename << "_" << original_filename.asASCIICharArray();
            pipeline_files.compute_shader << fixed_filename << "_" << original_filename.asASCIICharArray();
        }
        else if (!default_suffix.empty())
        {
            pipeline_files.vertex_shader << "_" << default_suffix.c_str();
            pipeline_files.tessellation_control_shader << "_" << default_suffix.c_str();
            pipeline_files.tessellation_evaluation_shader << "_" << default_suffix.c_str();
            pipeline_files.geometry_shader << "_" << default_suffix.c_str();
            pipeline_files.fragment_shader << "_" << default_suffix.c_str();
            pipeline_files.compute_shader << "_" << default_suffix.c_str();
        }

        // Stage token.
        pipeline_files.vertex_shader << "_" << kStrVertexStageNameAbbreviation;
        pipeline_files.tessellation_control_shader << "_" << kStrTessellationControlStageNameAbbreviation;
        pipeline_files.tessellation_evaluation_shader << "_" << kStrTessellationEvaluationStageNameAbbreviation;
        pipeline_files.geometry_shader << "_" << kStrGeometryStageNameAbbreviation;
        pipeline_files.fragment_shader << "_" << kStrFragmentStageNameAbbreviation;
        pipeline_files.compute_shader << "_" << kStrComputeStageNameAbbreviation;

        pipeline_files.vertex_shader << "." << (original_file_extension.isEmpty() ? default_ext.c_str() : original_file_extension.asASCIICharArray());
        pipeline_files.tessellation_control_shader << "." << (original_file_extension.isEmpty() ? default_ext.c_str() : original_file_extension.asASCIICharArray());
        pipeline_files.tessellation_evaluation_shader << "." << (original_file_extension.isEmpty() ? default_ext.c_str() : original_file_extension.asASCIICharArray());
        pipeline_files.geometry_shader << "." << (original_file_extension.isEmpty() ? default_ext.c_str() : original_file_extension.asASCIICharArray());
        pipeline_files.fragment_shader << "." << (original_file_extension.isEmpty() ? default_ext.c_str() : original_file_extension.asASCIICharArray());
        pipeline_files.compute_shader << "." << (original_file_extension.isEmpty() ? default_ext.c_str() : original_file_extension.asASCIICharArray());
    }

    return status;
}

std::string KcUtils::DeviceStatisticsToCsvString(const Config& config, const std::string& device, const beKA::AnalysisData& statistics)
{
    std::stringstream output;

    // Device name.
    char csv_separator = GetCsvSeparator(config);
    output << device << csv_separator;

    // Scratch registers.
    output << statistics.scratch_memory_used << csv_separator;

    // Work-items per work-group.
    output << statistics.num_threads_per_group_total << csv_separator;

    // Wavefront size.
    output << statistics.wavefront_size << csv_separator;

    // LDS available bytes.
    output << statistics.lds_size_available << csv_separator;

    // LDS actual bytes.
    output << statistics.lds_size_used << csv_separator;

    // Available SGPRs.
    output << statistics.num_sgprs_available << csv_separator;

    // Used SGPRs.
    output << statistics.num_sgprs_used << csv_separator;

    // Spills of SGPRs.
    output << statistics.num_sgpr_spills << csv_separator;

    // Available VGPRs.
    output << statistics.num_vgprs_available << csv_separator;

    // Used VGPRs.
    output << statistics.num_vgprs_used << csv_separator;

    // Spills of VGPRs.
    output << statistics.num_vgpr_spills << csv_separator;

    // CL Work-group dimensions (for a unified format, to be revisited).
    output << statistics.num_threads_per_group_x << csv_separator;
    output << statistics.num_threads_per_group_y << csv_separator;
    output << statistics.num_threads_per_group_z << csv_separator;

    // ISA size.
    output << statistics.isa_size;

    output << std::endl;

    return output.str().c_str();
}

bool KcUtils::CreateStatisticsFile(const gtString& filename, const Config& config,
                                   const std::map<std::string, beKA::AnalysisData>& analysis_data, LoggingCallbackFunction log_callback)
{
    bool ret = false;

    // Get the separator for CSV list items.
    char csv_separator = GetCsvSeparator(config);

    // Open output file.
    std::ofstream output;
    output.open(filename.asASCIICharArray());

    if (output.is_open())
    {
        // Write the header.
        output << GetStatisticsCsvHeaderString(csv_separator) << std::endl;

        // Write the device data.
        for (const auto& device_stats_pair : analysis_data)
        {
            // Write a line of CSV.
            output << DeviceStatisticsToCsvString(config, device_stats_pair.first, device_stats_pair.second);
        }

        output.close();
        ret = true;
    }
    else if (log_callback != nullptr)
    {
        std::stringstream log;
        log << kStrErrorCannotOpenFileForWriteA << filename.asASCIICharArray() <<
              kStrErrorCannotOpenFileForWriteB << std::endl;
        log_callback(log.str());
    }

    return ret;
}

std::string KcUtils::GetStatisticsCsvHeaderString(char csv_separator)
{
    // CSV file.
    static const char* kStrInfoCsvHeaderDevice = "DEVICE";
    static const char* kStrInfoCsvScratchMemory = "SCRATCH_MEM";
    static const char* kStrInfoCsvHeaderThreadsPerWorkgroup = "THREADS_PER_WORKGROUP";
    static const char* kStrInfoCsvHeaderWavefrontSize = "WAVEFRONT_SIZE";
    static const char* kStrInfoCsvHeaderLdsBytesMax = "AVAILABLE_LDS_BYTES";
    static const char* kStrInfoCsvHeaderLdsBytesActual = "USED_LDS_BYTES";
    static const char* kStrInfoCsvHeaderSgprAvailable = "AVAILABLE_SGPRs";
    static const char* kStrInfoCsvHeaderSgprUsed = "USED_SGPRs";
    static const char* kStrInfoCsvHeaderSgprSpills = "SGPR_SPILLS";
    static const char* kStrInfoCsvHeaderVgprAvailable = "AVAILABLE_VGPRs";
    static const char* kStrInfoCsvHeaderVgprUsed = "USED_VGPRs";
    static const char* kStrInfoCsvHeaderVgprSpills = "VGPR_SPILLS";
    static const char* kStrInfoCsvHeaderOpenclWorkgroupDimensionX = "CL_WORKGROUP_X_DIMENSION";
    static const char* kStrInfoCsvHeaderOpenclWorkgroupDimensionY = "CL_WORKGROUP_Y_DIMENSION";
    static const char* kStrInfoCsvHeaderOpenclWorkgroupDimensionZ = "CL_WORKGROUP_Z_DIMENSION";
    static const char* kStrInfoCsvHeaderIsaSizeBytes = "ISA_SIZE";

    std::stringstream output;
    output << kStrInfoCsvHeaderDevice << csv_separator;
    output << kStrInfoCsvScratchMemory << csv_separator;
    output << kStrInfoCsvHeaderThreadsPerWorkgroup << csv_separator;
    output << kStrInfoCsvHeaderWavefrontSize << csv_separator;
    output << kStrInfoCsvHeaderLdsBytesMax << csv_separator;
    output << kStrInfoCsvHeaderLdsBytesActual << csv_separator;
    output << kStrInfoCsvHeaderSgprAvailable << csv_separator;
    output << kStrInfoCsvHeaderSgprUsed << csv_separator;
    output << kStrInfoCsvHeaderSgprSpills << csv_separator;
    output << kStrInfoCsvHeaderVgprAvailable << csv_separator;
    output << kStrInfoCsvHeaderVgprUsed << csv_separator;
    output << kStrInfoCsvHeaderVgprSpills << csv_separator;
    output << kStrInfoCsvHeaderOpenclWorkgroupDimensionX << csv_separator;
    output << kStrInfoCsvHeaderOpenclWorkgroupDimensionY << csv_separator;
    output << kStrInfoCsvHeaderOpenclWorkgroupDimensionZ << csv_separator;
    output << kStrInfoCsvHeaderIsaSizeBytes;
    return output.str().c_str();
}

void KcUtils::CreateStatisticsFile(const gtString& filename, const Config& config, const std::string& device,
                                   const beKA::AnalysisData& device_statistics, LoggingCallbackFunction log_callback)
{
    // Create a temporary map and invoke the general routine.
    std::map<std::string, beKA::AnalysisData> tmp_map;
    tmp_map[device] = device_statistics;
    CreateStatisticsFile(filename, config, tmp_map, log_callback);
}

char KcUtils::GetCsvSeparator(const Config& config)
{
    char csv_separator;
    if (!config.csv_separator.empty())
    {
        csv_separator = config.csv_separator[0];
        if (config.csv_separator[0] == '\\' && config.csv_separator.size() > 1)
        {
            switch (config.csv_separator[1])
            {
                case 'a': csv_separator = '\a'; break;

                case 'b': csv_separator = '\b'; break;

                case 'f': csv_separator = '\f'; break;

                case 'n': csv_separator = '\n'; break;

                case 'r': csv_separator = '\r'; break;

                case 't': csv_separator = '\t'; break;

                case 'v': csv_separator = '\v'; break;

                default:
                    csv_separator = config.csv_separator[1];
                    break;
            }
        }
    }
    else
    {
        // The default separator.
        csv_separator = ',';
    }

    return csv_separator;
}

bool KcUtils::DeleteFile(const gtString& file_full_path)
{
    bool ret = false;
    osFilePath path(file_full_path);

    if (path.exists())
    {
        osFile file(path);
        ret = file.deleteFile();
    }
    return ret;
}

bool KcUtils::DeleteFile(const std::string& file_full_path)
{
    gtString path_gtstr;
    path_gtstr << file_full_path.c_str();
    return DeleteFile(path_gtstr);
}

bool KcUtils::IsDirectory(const std::string& dir_path)
{
    gtString path_gtstr;
    path_gtstr << dir_path.c_str();
    osFilePath file_path;
    file_path.setFullPathFromString(path_gtstr);
    return file_path.isDirectory();
}

void KcUtils::ReplaceStatisticsFile(const gtString& statistics_file, const Config& config,
                                    const std::string& device, IStatisticsParser& stats_parser, LoggingCallbackFunction log_cb)
{
    // Parse the backend statistics.
    beKA::AnalysisData statistics;
    stats_parser.ParseStatistics(device, statistics_file, statistics);

    // Delete the older statistics file.
    KcUtils::DeleteFile(statistics_file);

    // Create a new statistics file in the CLI format.
    KcUtils::CreateStatisticsFile(statistics_file, config, device, statistics, log_cb);
}

// Evaluates the result by a backend analysis session. Prints the relevant message.
// Returns true if analysis succeeded, false otherwise.
static bool EvaluateAnalysisResult(beStatus rc, LoggingCallbackFunction callback)
{
    if (rc != kBeStatusSuccess)
    {
        // Inform the user in case of an error.
        std::stringstream msg;

        switch (rc)
        {
        case beKA::kBeStatusShaeCannotLocateAnalyzer:
            // Failed to locate the ISA analyzer.
            msg << kStrErrorCannotLocateLiveregAnalyzer << std::endl;
            break;

        case beKA::kBeStatusShaeIsaFileNotFound:
            // ISA file not found.
            msg << kStrErrorCannotFindIsaFile << std::endl;
            break;

        case beKA::kBeStatusShaeFailedToLaunch:
#ifndef __linux__
            // Failed to launch the ISA analyzer.
            // On Linux, there is an issue with this return code due to the
            // executable format that we use for the backend.
            msg << kStrErrorCannotLaunchLiveregAnalyzer << std::endl;
#endif
            break;

        case beKA::kBeStatusGeneralFailed:
        default:
            // Generic error message.
            msg << kStrErrorCannotPerformLiveregAnalysis << std::endl;
            break;
        }

        const std::string& error_msg = msg.str();
        if (!error_msg.empty())
        {
            if (callback != nullptr)
            {

                callback(error_msg);
            }
            else
            {
                std::cout << error_msg << std::endl;
            }
        }
    }

    return (rc == kBeStatusSuccess);
}

bool KcUtils::PerformLiveRegisterAnalysis(const gtString& isa_filename, const gtString& target, const gtString& output_filename,
                                          LoggingCallbackFunction callback, bool print_cmd)
{
    // Call the backend.
    beStatus rc = BeStaticIsaAnalyzer::PerformLiveRegisterAnalysis(isa_filename, target, output_filename, print_cmd);

    return EvaluateAnalysisResult(rc, callback);
}

bool KcUtils::PerformLiveRegisterAnalysis(const std::string& isa_filename, const std::string& target,
    const std::string& output_filename, LoggingCallbackFunction callback, bool print_cmd)
{
    // Convert the arguments to gtString.
    gtString isa_name_gtstr;
    isa_name_gtstr << isa_filename.c_str();
    gtString output_filename_gtstr;
    output_filename_gtstr << output_filename.c_str();
    gtString target_gtstr;
    target_gtstr << target.c_str();

    // Invoke the routine.
    return PerformLiveRegisterAnalysis(isa_name_gtstr, target_gtstr, output_filename_gtstr, callback, print_cmd);
}

bool KcUtils::PerformStallAnalysis(const std::string& isa_filename,const std::string& target, const std::string& output_filename, LoggingCallbackFunction callback, bool print_cmd)
{
    // Convert the arguments to gtString.
    gtString isa_name_gtstr;
    isa_name_gtstr << isa_filename.c_str();
    gtString output_filename_gtstr;
    output_filename_gtstr << output_filename.c_str();
    gtString target_gtstr;
    target_gtstr << target.c_str();

    // Invoke the routine.
    return PerformStallAnalysis(isa_name_gtstr, target_gtstr, output_filename_gtstr, callback, print_cmd);
}

bool KcUtils::PerformStallAnalysis(const gtString& isa_filename, const gtString& target, const gtString& output_filename, LoggingCallbackFunction callback, bool print_cmd)
{
    // Call the backend.
    beStatus rc = BeStaticIsaAnalyzer::PerformStallAnalysis(isa_filename, target, output_filename, print_cmd);

    return EvaluateAnalysisResult(rc, callback);
}

bool KcUtils::GenerateControlFlowGraph(const gtString& isa_file_name, const gtString& target, const gtString& output_filename,
                                       LoggingCallbackFunction callback, bool per_inst_cfg, bool printCmd)
{
    // Call the backend.
    beStatus rc = BeStaticIsaAnalyzer::GenerateControlFlowGraph(isa_file_name, target, output_filename, per_inst_cfg, printCmd);
    if (rc != kBeStatusSuccess && callback != nullptr)
    {
        // Inform the user in case of an error.
        std::stringstream msg;

        switch (rc)
        {
            case beKA::kBeStatusShaeCannotLocateAnalyzer:
                // Failed to locate the ISA analyzer.
                msg << kStrErrorCannotLocateLiveregAnalyzer << std::endl;
                break;

            case beKA::kBeStatusShaeIsaFileNotFound:
                // ISA file not found.
                msg << kStrErrorCannotFindIsaFile << std::endl;
                break;

            case beKA::kBeStatusShaeFailedToLaunch:
#ifndef __linux__
                // Failed to launch the ISA analyzer.
                // On Linux, there is an issue with this return code due to the
                // executable format that we use for the backend.
                msg << kStrErrorCannotLaunchCfgAnalyzer << std::endl;
#endif
                break;

            case beKA::kBeStatusGeneralFailed:
            default:
                // Generic error message.
                msg << kStrErrorCannotPerformLiveregAnalysis << std::endl;
                break;
        }

        const std::string& error_msg = msg.str();
        if (!error_msg.empty() && callback != nullptr)
        {
            callback(error_msg);
        }
    }

    return (rc == kBeStatusSuccess);
}

bool KcUtils::GenerateControlFlowGraph(const std::string& isa_filename, const std::string& target, const std::string& output_filename,
    LoggingCallbackFunction pCallback, bool per_inst_cfg, bool print_cmd)
{
    // Convert the arguments to gtString.
    gtString isa_name_gtstr;
    isa_name_gtstr << isa_filename.c_str();
    gtString output_filename_gtstr;
    output_filename_gtstr << output_filename.c_str();
    gtString target_gtstr;
    target_gtstr << target.c_str();

    // Invoke the routine.
    return GenerateControlFlowGraph(isa_name_gtstr, target_gtstr, output_filename_gtstr, pCallback, per_inst_cfg, print_cmd);
}

void KcUtils::ConstructOutputFileName(const std::string& base_output_filename, const std::string& default_suffix,
                                      const std::string& default_extension, const std::string& kernel_name,
                                      const std::string& device_name, gtString& generated_filename)
{
    // Convert the base output file name to gtString.
    gtString base_output_filename_as_gtstr;
    base_output_filename_as_gtstr << base_output_filename.c_str();
    osFilePath output_file_path(base_output_filename_as_gtstr);

    // Extract the user's file name and extension.
    gtString filename;
    if (!output_file_path.isDirectory())
    {
        output_file_path.getFileName(filename);
    }
    else
    {
        osDirectory output_dir(base_output_filename_as_gtstr);
        output_file_path.setFileDirectory(output_dir);
    }

    // Fix the user's file name to generate a unique output file name in the Analyzer CLI format.
    gtString fixed_filename, suffix;
    fixed_filename << device_name.c_str();
    suffix << default_suffix.c_str();

    if (!kernel_name.empty())
    {
        if (!fixed_filename.isEmpty())
        {
            fixed_filename << "_";
        }

        fixed_filename << kernel_name.c_str();
    }

    if (!suffix.isEmpty() || !filename.isEmpty())
    {
        fixed_filename << (fixed_filename.isEmpty() ? "" : "_") << (filename.isEmpty() ? suffix : filename);
    }

    output_file_path.setFileName(fixed_filename);

    // Handle the default extension (unless the user specified an extension).
    gtString outputFileExtension;
    output_file_path.getFileExtension(outputFileExtension);

    if (outputFileExtension.isEmpty())
    {
        outputFileExtension.fromASCIIString(default_extension.c_str());
        output_file_path.setFileExtension(outputFileExtension);
    }

    // Set the output string.
    generated_filename = output_file_path.asString();
}

void KcUtils::ConstructOutputFileName(const std::string& base_output_file_name, const std::string& default_suffix,
                                      const std::string& default_extension, const std::string& entry_point_name,
                                      const std::string& device_name, std::string& generated_filename)
{
    gtString  out_filename_gtstr;
    ConstructOutputFileName(base_output_file_name, default_suffix, default_extension, entry_point_name, device_name, out_filename_gtstr);
    generated_filename = out_filename_gtstr.asASCIICharArray();
}

bool KcUtils::ConstructOutFileName(const std::string& base_filename, const std::string& stage,
    const std::string& device, const std::string& ext, std::string& out_filename, bool should_append_suffix)
{
    static const std::string  STR_TEMP_FILE_NAME = "rga-temp-out";
    bool status = false;
    gtString name = L"";
    std::string base_name = base_filename;

    // If base output file name is not provided, create a temp file name (in the temp folder).
    if (!base_name.empty())
    {
        status = true;
    }
    else
    {
        base_name = KcUtils::ConstructTempFileName(STR_TEMP_FILE_NAME, ext);
        status = !base_name.empty();
        if (!status)
        {
            RgLog::stdOut << kStrErrorFailedCreateOutputFilename << std::endl;
        }
    }

    if (status)
    {
        std::string out_name;
        KcUtils::ConstructOutputFileName(base_name, stage, ext, "", device, out_name);
        if (!out_name.empty())
        {
            // Add the wildcard token if it isn't already baked into the file name.
            const char* kWildcardToken = "*";
            if (should_append_suffix && out_name.find(kWildcardToken) == std::string::npos)
            {
                KcUtils::AppendSuffix(out_name, stage);
            }

            // We are done.
            out_filename = out_name;
            status = true;
        }
        else
        {
            RgLog::stdOut << kStrErrorFailedCreateOutputFilename << std::endl;
        }
    }

    return status;
}

void KcUtils::AppendSuffix(std::string& filename, const std::string& suffix)
{
    if (!suffix.empty())
    {
        gtString filename_gtstr, suffix_gtstr;
        filename_gtstr << filename.c_str();
        suffix_gtstr << suffix.c_str();
        osFilePath filePath(filename_gtstr);
        gtString base_filename;
        filePath.getFileName(base_filename);
        base_filename += L"_";
        base_filename += suffix_gtstr;
        filePath.setFileName(base_filename);
        filename = filePath.asString().asASCIICharArray();
    }
}

gtString KcUtils::ConstructTempFileName(const gtString& prefix, const gtString & ext)
{
    const unsigned int kMAX_ATTEMPTS = 1024;
    osDirectory  rga_temp_dir;
    gtString  ret = L"";

    if (GetRGATempDir(rga_temp_dir))
    {
        if (!rga_temp_dir.exists())
        {
            rga_temp_dir.create();
        }

        osFilePath temp_file_path;
        temp_file_path.setFileDirectory(rga_temp_dir);

        gtString temp_file_base_name = prefix;
        temp_file_base_name.appendUnsignedIntNumber(osGetCurrentProcessId());
        gtString temp_filename = temp_file_base_name;
        temp_filename.append(L".");
        temp_filename.append(ext);
        temp_file_path.setFileName(temp_filename);

        // Ensure that the file name is unique for any invocation by the existing CLI process.
        static unsigned unique_suffix = 0;

        uint32_t suffix_num = 0;
        while (temp_file_path.exists() && suffix_num < kMAX_ATTEMPTS)
        {
            temp_filename = temp_file_base_name;
            temp_filename.appendUnsignedIntNumber(suffix_num++);
            temp_filename.appendUnsignedIntNumber(unique_suffix++);
            temp_filename.append(L".");
            temp_filename.append(ext);
            temp_file_path.setFileName(temp_filename);
        }

        ret = suffix_num < kMAX_ATTEMPTS ? temp_file_path.asString() : L"";
    }

    return ret;
}

std::string KcUtils::ConstructTempFileName(const std::string & prefix, const std::string & ext)
{
    gtString prefix_gtstr, ext_gtstr, filename_gtstr;
    prefix_gtstr << prefix.c_str();
    ext_gtstr << ext.c_str();
    filename_gtstr = ConstructTempFileName(prefix_gtstr, ext_gtstr);
    return filename_gtstr.asASCIICharArray();
}

bool KcUtils::GetMarketingNameToCodenameMapping(DeviceNameMap& cards_map)
{
    bool ret = BeUtils::GetMarketingNameToCodenameMapping(cards_map);

    // Remove the disabled devices.
    for (const std::string& disabledDevice : kRgaDisabledDevices)
    {
        cards_map.erase(disabledDevice);
    }

    return ret;
}

// Converts "srcName" string to lowercase and removes all spaces and '-'.
// Stores the result in "dstName".
static void ReduceDeviceName(std::string& name)
{
    std::transform(name.begin(), name.end(), name.begin(), [](const char& c) {return std::tolower(c);});
    name.erase(std::remove_if(name.begin(), name.end(), [](const char& c) {return (std::isspace(c) || c == '-');}), name.end());
}

// Helper function that interprets the matched devices found by the "KcUtils::FindGPUArchName()".
static bool ResolveMatchedDevices(const KcUtils::DeviceNameMap& matched_devices, const std::string& device,
                                  bool print_info, bool print_unknown_device_error)
{
    bool status = false;
    std::stringstream out_msg, error_msg;

    // Routine printing the architecture name and all its device names to required stream.
    auto print_arch_and_devices = [&](const KcUtils::DeviceNameMap::value_type& arch, std::stringstream& s)
    {
        s << arch.first << std::endl;
        for (const std::string& marketing_name : arch.second)
        {
            s << "\t" << marketing_name << std::endl;
        }
    };

    if (matched_devices.size() == 0)
    {
        if (print_info && print_unknown_device_error)
        {
            // No matching architectures found. Failure.
            error_msg << kStrErrorCouldNotDetectTarget << device << std::endl;
        }
    }
    else if (matched_devices.size() == 1)
    {
        // Found exactly one GPU architecture. Success.
        if (print_info)
        {
            out_msg << STR_TARGET_DETECTED << std::endl << std::endl;
            print_arch_and_devices(*(matched_devices.begin()), out_msg);
            out_msg << std::endl;
        }
        status = true;
    }
    else if (print_info)
    {
        // Found multiple GPU architectures. Failure.
        error_msg << kStrErrorAmbiguousTarget << device << std::endl << std::endl;
        for (auto& arch : matched_devices)
        {
            print_arch_and_devices(arch, error_msg);
        }
    }

    RgLog::stdOut << out_msg.str() << RgLog::flush;
    if (!error_msg.str().empty())
    {
        RgLog::stdErr << error_msg.str() << std::endl;
    }

    return status;
}

bool KcUtils::FindGPUArchName(const std::string& device, std::string& matched_device, bool print_info, bool allow_unknown_device)
{
    bool status = false;
    const char* kFILTER_INDICATOR_1 = ":";
    const char* kFILTER_INDICATOR_2 = "Not Used";

    // Mappings  "architecture <--> marketing names"
    DeviceNameMap matched_devices, cards_mapping;

    // Transform the device name to lower case and remove spaces.
    std::string reduced_name = device;
    ReduceDeviceName(reduced_name);

    bool rc = KcUtils::GetMarketingNameToCodenameMapping(cards_mapping);

    // Walk over all known architectures and devices.
    if (rc && !cards_mapping.empty())
    {
        for (const auto& pair : cards_mapping)
        {
            const std::string& arch_name = pair.first;
            std::string  reduced_arch_name = arch_name;
            ReduceDeviceName(reduced_arch_name);

            // If we found a match with an arch name -- add it to the list of matched archs and continue.
            // Otherwise, look at the marketing device names.
            if (reduced_arch_name.find(reduced_name) != std::string::npos)
            {
                // Put the found arch with an all its devices into the "matchedArchs" map.
                // Filter out the devices with code names and unused names.
                std::set<std::string>& device_list = matched_devices[arch_name];
                for (const std::string& device : pair.second)
                {
                    if (device.find(kFILTER_INDICATOR_1) == std::string::npos && device.find(kFILTER_INDICATOR_2) == std::string::npos)
                    {
                        device_list.emplace(device);
                    }
                }
            }
            else
            {
                bool  added_arch = false;
                for (const std::string& marketing_name : pair.second)
                {
                    // We do not want to display names that contain these strings.
                    if (marketing_name.find(kFILTER_INDICATOR_1) ==
                        std::string::npos && marketing_name.find(kFILTER_INDICATOR_2) == std::string::npos)
                    {
                        std::string reduced_marketing_name = marketing_name;
                        ReduceDeviceName(reduced_marketing_name);

                        if (reduced_marketing_name.find(reduced_name) != std::string::npos)
                        {
                            // Found a match with the marketing name -- add it to the device list corresponding to the "archName" architecture.
                            if (!added_arch)
                            {
                                matched_devices[arch_name] = std::set<std::string>();
                                added_arch = true;
                            }
                            matched_devices[arch_name].emplace(marketing_name);
                        }
                    }
                }
            }
        }
    }

    // Interpret the matched names.
    if (ResolveMatchedDevices(matched_devices, device, print_info, !allow_unknown_device))
    {
        matched_device = (*(matched_devices.begin())).first;
        status = true;
    }

    return status;
}

// Returns a subdirectory of the OS temp directory where RGA keeps all temporary files.
// Creates the subdirectory if it does not exists.
bool GetRGATempDir(osDirectory & dir)
{
    const gtString kAMD_RGA_TEMP_DIR_1 = L"GPUOpen";
    const gtString kAMD_RGA_TEMP_DIR_2 = L"rga";
    bool  ret = true;

    osFilePath temp_dir_path(osFilePath::OS_TEMP_DIRECTORY);
    temp_dir_path.appendSubDirectory(kAMD_RGA_TEMP_DIR_1);
    temp_dir_path.getFileDirectory(dir);
    if (!dir.exists())
    {
        ret = dir.create();
    }

    if (ret)
    {
        temp_dir_path.appendSubDirectory(kAMD_RGA_TEMP_DIR_2);
        temp_dir_path.getFileDirectory(dir);
        if (!dir.exists())
        {
            ret = dir.create();
        }
    }

    return ret;
}

bool KcUtils::PrintAsicList(const std::set<std::string>& required_devices, const std::set<std::string>& disabled_devices)
{
    // We do not want to display names that contain these strings.
    const char* kFILTER_INDICATOR_1 = ":";
    const char* kFILTER_INDICATOR_2 = "Not Used";

    bool result = false;
    std::map<std::string, std::set<std::string>> cards_mapping;
    bool rc = KcUtils::GetMarketingNameToCodenameMapping(cards_mapping);

    // Sort the mappings.
    std::map<std::string, std::set<std::string>,
        decltype(&BeUtils::DeviceNameLessThan)> cards_mapping_sorted(cards_mapping.begin(),
        cards_mapping.end(), &BeUtils::DeviceNameLessThan);

    if (rc && !cards_mapping_sorted.empty())
    {
        for (const auto& pair : cards_mapping_sorted)
        {
            // If "reqdDevices" is provided, print only devices from this set.
            // If "disdDevices" is provided, do not pring devices from this set.
            // The "reqdDevices" contains short arch names (like "gfx804"), while
            // the container has extended names: "gfx804 (Graphics IP v8)".
            auto is_in_device_list = [](const std::set<std::string>& list, const std::string & device)
            {
                for (auto & d : list)
                    if (KcUtils::ToLower(device).find(KcUtils::ToLower(d)) != std::string::npos)
                        return true;
                return false;
            };

            if ((required_devices.empty() || is_in_device_list(required_devices, pair.first)) &&
                (disabled_devices.empty() || !is_in_device_list(disabled_devices, pair.first)))
            {
                RgLog::stdOut << RgLog::noflush << pair.first << std::endl;
                for (const std::string& card : pair.second)
                {
                    // Filter out internal names.
                    if (card.find(kFILTER_INDICATOR_1) == std::string::npos &&
                        card.find(kFILTER_INDICATOR_2) == std::string::npos)
                    {
                        RgLog::stdOut << "\t" << card << std::endl;
                    }
                }
                RgLog::stdOut << RgLog::flush;
            }
        }
        result = true;
    }

    return result;
}

bool KcUtils::GetLogFileName(std::string& log_filename)
{
    bool ret = false;
    osDirectory tmp_dir;
    osFilePath log_file;
    if (GetRGATempDir(tmp_dir))
    {
        log_file.setFileDirectory(tmp_dir);
        log_file.setFileName(kRgaCliLogFileName);
        log_file.setFileExtension(kRgaCliLogFileExt);
        log_filename = log_file.asString().asASCIICharArray();

        // Wrap the file name with double quotes in case that it
        // contains a whitespace.
        if (log_filename.find_first_not_of(' ') != std::string::npos)
        {
            std::stringstream wrapped_filename;
            wrapped_filename << "\"" << log_filename << "\"";
            log_filename = wrapped_filename.str();
        }

        ret = true;
    }
    return ret;
}

bool KcUtils::GetParsedISAFileName(const std::string& isa_filename, std::string& parsed_isa_filename)
{
    gtString  filename_gtstr;
    filename_gtstr.fromASCIIString(isa_filename.c_str());
    osFilePath filePath(filename_gtstr);
    filePath.setFileExtension(kRgaCliParsedIsaFileExt);
    parsed_isa_filename = filePath.asString().asASCIICharArray();
    return (!parsed_isa_filename.empty());
}

std::string KcUtils::Quote(const std::string& str)
{
    return (str.find(' ') == std::string::npos ? str : (std::string("\"") + str + '"'));
}

std::string KcUtils::ToLower(const std::string& str)
{
    std::string lstr = str;
    std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](const char& c) {return std::tolower(c);});
    return lstr;
}

void KcUtils::DeletePipelineFiles(const BeProgramPipeline & files)
{
    auto deleteFile = [](const gtString& filename)
                      { if (!filename.isEmpty() && FileNotEmpty(filename.asASCIICharArray())) { KcUtils::DeleteFile(filename); } };

    deleteFile(files.vertex_shader);
    deleteFile(files.tessellation_control_shader);
    deleteFile(files.tessellation_evaluation_shader);
    deleteFile(files.geometry_shader);
    deleteFile(files.fragment_shader);
    deleteFile(files.compute_shader);
}

const std::vector<std::string> KcUtils::GetRgaDisabledDevices()
{
    return kRgaDisabledDevices;
}

void KcUtils::PrintRgaVersion()
{
    std::cout << kStrRgaProductName << " " << kStrRgaVersionPrefix << kStrRgaVersion << "." << kStrRgaBuildNum << std::endl;
}

#ifdef _WIN32
KcUtils::ProcessStatus KcUtils::LaunchProcess(const std::string& exec_path, const std::string& args, const std::string& dir,
    unsigned long time_out, bool print_cmd, std::string& std_out, std::string& std_err, long& exit_code)
{
    ProcessStatus status = ProcessStatus::kSuccess;
    exit_code = 0;

    // Set working directory, executable and arguments.
    osFilePath  work_dir;
    if (dir == "")
    {
        work_dir.setPath(osFilePath::OS_CURRENT_DIRECTORY);
    }
    else
    {
        gtString dir_gtstr;
        dir_gtstr << dir.c_str();
        work_dir.setFileDirectory(dir_gtstr);
    }

    // Log the invocation event.
    std::stringstream msg;
    msg << kStrLaunchingExternalProcess << exec_path.c_str()
        << " " << args.c_str();

    RgLog::file << msg.str() << std::endl;
    if (print_cmd)
    {
        RgLog::stdOut << msg.str() << std::endl;
    }

    // Construct the invocation command.
    std::stringstream cmd;
    cmd << exec_path.c_str() << " " << args.c_str();

    // Launch the process.
    bool should_cancel = false;
    gtString working_dir = work_dir.asString();
    gtString cmd_output;
    gtString cmdOutputErr;
    bool is_launch_success = osExecAndGrabOutputAndError(cmd.str().c_str(), should_cancel,
        working_dir, cmd_output, cmdOutputErr);

    // Read stdout and stderr.
    std_out = cmd_output.asASCIICharArray();
    std_err = cmdOutputErr.asASCIICharArray();

    if (!is_launch_success)
    {
        status = ProcessStatus::kLaunchFailed;
    }

    return status;
}
#else
KcUtils::ProcessStatus KcUtils::LaunchProcess(const std::string& exec_path, const std::string& args, const std::string& dir,
    unsigned long time_out, bool print_cmd, std::string& std_out, std::string& std_err, long& exit_code)
{
    osProcessId      compiler_proc_id;
    osProcessHandle  compiler_proc_handle;
    osThreadHandle   compiler_thread_handle;
    ProcessStatus    status = ProcessStatus::kSuccess;

    // Set working directory, executable and arguments.
    osFilePath  work_dir;
    if (dir == "")
    {
        work_dir.setPath(osFilePath::OS_CURRENT_DIRECTORY);
    }
    else
    {
        gtString  dir_gtstr;
        dir_gtstr << dir.c_str();
        work_dir.setFileDirectory(dir_gtstr);
    }

    gtString exec_path_gtstr, args_gtstr;
    exec_path_gtstr << exec_path.c_str();
    args_gtstr << args.c_str();

    // Create temporary files for stdout/stderr.
    gtString  out_filename = KcUtils::ConstructTempFileName(kRgaCliTempStdoutFilename, kRgaCliTempStdoutFileExt);
    gtString  err_filename = KcUtils::ConstructTempFileName(kRgaCliTempStderrFilename, kRgaCliTempStderrFileExt);

    if (out_filename.isEmpty() || err_filename.isEmpty())
    {
        status = ProcessStatus::kCreateTempFileFailed;
    }
    else
    {
        std::stringstream msg;
        msg << kStrLaunchingExternalProcess << exec_path_gtstr.asASCIICharArray()
            << " " << args_gtstr.asASCIICharArray();

        RgLog::file << msg.str() << std::endl;

        if (print_cmd)
        {
            RgLog::stdOut << msg.str() << std::endl;
        }

        args_gtstr += L" >";
        args_gtstr += out_filename;
        args_gtstr += L" 2>";
        args_gtstr += err_filename;

        // Launch a process.
        bool  proc_status = osLaunchSuspendedProcess(exec_path_gtstr,
            args_gtstr,
            work_dir,
            compiler_proc_id,
            compiler_proc_handle,
            compiler_thread_handle,
            false,  // Don't create a window
            true);  // Redirect stdout & stderr

        if (proc_status && osResumeSuspendedProcess(compiler_proc_id, compiler_proc_handle, compiler_thread_handle, false))
        {
            proc_status = osWaitForProcessToTerminate(compiler_proc_id, time_out, &exit_code);

            if (!proc_status)
            {
                osTerminateProcess(compiler_proc_id);
                status = ProcessStatus::kTimeOut;
            }
        }
        else
        {
            status = ProcessStatus::kLaunchFailed;
        }
    }

    // Read the content of stderr and stdout files.
    std_out = std_err = "";
    if (status == ProcessStatus::kSuccess)
    {
        if (FileNotEmpty(out_filename.asASCIICharArray()))
        {
            if (!KcUtils::ReadTextFile(out_filename.asASCIICharArray(), std_out, nullptr))
            {
                status = ProcessStatus::kReadTempFileFailed;
            }
            std::remove(out_filename.asASCIICharArray());
        }

        if (FileNotEmpty(err_filename.asASCIICharArray()))
        {
            if (!KcUtils::ReadTextFile(err_filename.asASCIICharArray(), std_err, nullptr))
            {
                status = ProcessStatus::kReadTempFileFailed;
            }
            std::remove(err_filename.asASCIICharArray());
        }
    }

    return status;
}
#endif

bool KcUtils::FileNotEmpty(const std::string filename)
{
    bool  ret = false;
    if (!filename.empty())
    {
        std::ifstream file(filename);
        ret = (file.good() && file.peek() != std::ifstream::traits_type::eof());
    }
    return ret;
}

bool KcUtils::ReadProgramSource(const string& input_file, string& program_source)
{
    std::ifstream input;

#ifndef _WIN32
    // test if the input file is a directory.
    // On some Linux machine ifstream open will be valid if it is a directory
    // but will not allow to read data which will cause a crash when trying to read the data
    DIR* dir;
    dir = opendir(input_file.c_str());
    if (dir != NULL)
    {
        (void)closedir(dir);
        return false;
    }

#endif

    // Open (at e)nd of file.  We want the length below.
    // Open binary because it's faster & other bits of code deal OK with CRLF, LF etc.
    input.open(input_file.c_str(), ios::ate | ios::binary);

    if (!input)
    {
        return false;
    }

    std::ifstream::pos_type file_size = 0;
    file_size = input.tellg();
    if (file_size == static_cast<std::ifstream::pos_type>(0))
    {
        input.close();
        return false;
    }

    input.seekg(0, ios::beg);

    program_source.resize(size_t(file_size));
    input.read(&program_source[0], file_size);

    input.close();
    return true;
}

bool KcUtils::WriteBinaryFile(const std::string& filename, const std::vector<char>& content, LoggingCallbackFunction callback)
{
    bool ret = false;
    std::ofstream output;
    output.open(filename.c_str(), ios::binary);

    if (output.is_open() && !content.empty())
    {
        output.write(&content[0], content.size());
        output.close();
        ret = true;
    }
    else
    {
        std::stringstream log;
        log << "Error: Unable to open " << filename << " for write.\n";
        if (callback != nullptr)
        {
            callback(log.str());
        }
    }
    return ret;
}

bool KcUtils::ReadTextFile(const std::string& filename, std::string& content, LoggingCallbackFunction callback)
{
    bool ret = false;
    ifstream input;
    input.open(filename.c_str());

    if (input.is_open())
    {
        std::stringstream text_stream;
        text_stream << input.rdbuf();
        content = text_stream.str();
        input.close();
        ret = true;
    }
    else
    {
        std::stringstream log;
        log << kStrErrorCannotReadFile << filename << std::endl;
        if (callback != nullptr)
        {
            callback(log.str());
        }
    }
    return ret;
}

bool KcUtils::WriteTextFile(const std::string& filename, const std::string& content, LoggingCallbackFunction callback)
{
    bool ret = false;

    std::ofstream output;
    output.open(filename.c_str());

    if (output.is_open())
    {
        output << content << std::endl;
        output.close();
        ret = true;
    }
    else
    {
        std::stringstream log;
        log << kStrErrorCannotOpenFileForWriteA << filename << kStrErrorCannotOpenFileForWriteB << std::endl;
        if (callback != nullptr)
        {
            callback(log.str());
        }
    }

    return ret;
}

// Get current system time.
static bool CurrentTime(struct tm& time_buffer)
{
    bool ret = false;
    std::stringstream suffix;
    time_t  current_time = std::time(0);
#ifdef _WIN32
    struct tm* time_local = &time_buffer;
    ret = (localtime_s(time_local, &current_time) == 0);
#else
    struct tm* time_local = localtime(&current_time);
    if (time_local != nullptr)
    {
        time_buffer = *time_local;
        ret = true;
    }
#endif
    return ret;
}

// Delete log files older than 1 week.
static bool  DeleteOldLogs()
{
    bool  ret = false;
    const double kOneWeekSeconds = static_cast<double>(7*24*60*60);
    osDirectory tmp_dir;
    if ((ret = GetRGATempDir(tmp_dir)) == true)
    {
        gtString log_file_pattern;
        gtList<osFilePath> file_paths;
        log_file_pattern << kRgaCliLogFileName.asASCIICharArray() << "*." << kRgaCliLogFileExt.asASCIICharArray();
        if (tmp_dir.getContainedFilePaths(log_file_pattern, osDirectory::SORT_BY_DATE_ASCENDING, file_paths))
        {
            for (const osFilePath& path : file_paths)
            {
                osStatStructure file_stat;
                if ((ret = (osWStat(path.asString(), file_stat) == 0)) == true)
                {
                    time_t file_time = file_stat.st_ctime;
                    struct tm time;
                    if ((ret = CurrentTime(time)) == true)
                    {
                        time_t curr_time = std::mktime(&time);
                        if (std::difftime(curr_time, file_time) > kOneWeekSeconds)
                        {
                            ret = (std::remove(path.asString().asASCIICharArray()) == 0);
                        }
                    }
                }
            }
        }
    }
    return ret;
}

// Perform log file initialization.
bool  KcUtils::InitCLILogFile(const Config& config)
{
    bool status = DeleteOldLogs();
    if (!status)
    {
        RgLog::stdErr << kStrWarningFailedToDeleteLogFiles << std::endl;
    }

    std::string log_filename = config.log_file;
    if (log_filename.empty())
    {
        gtString filename_gtstr = KcUtils::ConstructTempFileName(kRgaCliLogFileName, kRgaCliLogFileExt);
        log_filename = filename_gtstr.asASCIICharArray();
    }

    if ((status = !log_filename.empty()) == true)
    {
        struct tm tt;
        status = CurrentTime(tt);
        auto zero_ext = [](int n) { std::string n_str = std::to_string(n); return (n < 10 ? std::string("0") + n_str : n_str); };

        // Add time prefix to the log file name if file name is not specified by the "--log" option.
        if (config.log_file.empty())
        {
            // Append current date/time to the log file name.
            std::stringstream suffix;
            suffix << "-" << std::to_string(tt.tm_year + 1900) << zero_ext(tt.tm_mon + 1) << zero_ext(tt.tm_mday) <<
                "-" << zero_ext(tt.tm_hour) << zero_ext(tt.tm_min) << zero_ext(tt.tm_sec);

            size_t  ext_offset = log_filename.rfind('.');
            log_filename.insert((ext_offset == std::string::npos ? log_filename.size() : ext_offset), suffix.str());
        }

        // Open log file.
        if ((status = RgLog::OpenLogFile(log_filename)) == true)
        {
            RgLog::file << kStrRgaCliLogsStart << std::endl;
        }
        else
        {
            RgLog::stdErr << kStrErrorFailedToOpenLogFile << log_filename << std::endl;
        }
    }

    return status;
}

bool KcUtils::StrCmpNoCase(const std::string & s1, const std::string & s2)
{
    std::string s1_u = s1, s2_u = s2;
    std::transform(s1_u.begin(), s1_u.end(), s1_u.begin(), [](unsigned char c) {return std::toupper(c);});
    std::transform(s2_u.begin(), s2_u.end(), s2_u.begin(), [](unsigned char c) {return std::toupper(c);});
    return (s1_u == s2_u);
}

std::string KcUtils::GetFileExtension(const std::string & file_path)
{
    // Try deducing from the extension.
    gtString  gfilePath;
    gfilePath << file_path.c_str();
    osFilePath path(gfilePath);
    gtString file_ext_gtstr;
    return (path.getFileExtension(file_ext_gtstr) ? file_ext_gtstr.asASCIICharArray() : "");
}

bool KcUtils::SetEnvrironmentVariable(const::std::string& var_name, const std::string var_value)
{
    // Convert to gtString.
    gtString var_name_gtstr;
    gtString var_value_gtstr;
    var_name_gtstr << var_name.c_str();
    var_value_gtstr << var_value.c_str();

    // Set the environment variable.
    osEnvironmentVariable env_var_vk_loader_debug(var_name_gtstr, var_value_gtstr);
    bool ret = osSetCurrentProcessEnvVariable(env_var_vk_loader_debug);

    return ret;
}

void KcUtils::CheckForUpdates()
{
    UpdateCheck::VersionInfo rga_cli_version;
    std::string build_date_string(kStrRgaBuildDate);

    if (build_date_string == kStrRgaBuildDateDev)
    {
        // Pretend a dev build has no version so that
        // all public versions are reported as being newer.
        rga_cli_version.major = 0;
        rga_cli_version.minor = 0;
        rga_cli_version.patch = 0;
        rga_cli_version.build = 0;
    }
    else
    {
        rga_cli_version.major = RGA_VERSION_MAJOR;
        rga_cli_version.minor = RGA_VERSION_MINOR;
        rga_cli_version.patch = RGA_VERSION_UPDATE;
        rga_cli_version.build = 0;
    }

    UpdateCheck::UpdateInfo update_info;
    string error_message;

    std::cout << "Checking for update... ";

    bool checked_for_update = UpdateCheck::CheckForUpdates(rga_cli_version, kStrRgaUpdatecheckUrl, kStrRgaUpdatecheckAssetName, update_info, error_message);
    if (!checked_for_update)
    {
        std::cout << "Unable to find update: " << error_message << endl;
    }
    else
    {
        if (!update_info.is_update_available)
        {
            std::cout << "No new updates available." << endl;
        }
        else
        {
            std::cout << "New version available!" << endl;

            for (std::vector<UpdateCheck::ReleaseInfo>::const_iterator release_iter = update_info.releases.cbegin(); release_iter != update_info.releases.cend(); ++release_iter)
            {
                std::cout << "Description: " << release_iter->title << endl;
                std::cout << "Version: " << release_iter->version.ToString() << " (" << UpdateCheck::ReleaseTypeToString(release_iter->type) << ")" << endl;
                std::cout << "Released: " << release_iter->date << endl;

                if (!release_iter->tags.empty())
                {
                    std::cout << "Tags: " << release_iter->tags[0];
                    for (uint32_t i = 1; i < release_iter->tags.size(); ++i)
                    {
                        std::cout << ", " << release_iter->tags[i];
                    }
                    std::cout << endl;
                }

                if (release_iter->download_links.size() > 0)
                {
                    std::cout << "Download a package from:" << endl;

                    for (size_t package_index = 0; package_index < release_iter->download_links.size(); ++package_index)
                    {
                        std::cout << "   " << release_iter->download_links[package_index].url << endl;
                    }
                }

                if (release_iter->info_links.size() > 0)
                {
                    std::cout << "For more information, visit:" << endl;

                    for (size_t infoLinkIndex = 0; infoLinkIndex < release_iter->info_links.size(); ++infoLinkIndex)
                    {
                        std::cout << "   " << release_iter->info_links[infoLinkIndex].url << endl;
                    }
                }

                // Add an extra line at the end in case there are multiple versions available.
                std::cout << endl;
            }
        }
    }
}

bool KcUtils::IsNaviTarget(const std::string& target_name)
{
    // Token to identify Navi targets.
    static const char* kNaviTargetToken = "gfx1";
    return (target_name.find(kNaviTargetToken) != std::string::npos);
}

bool KcUtils::IsNavi21AndBeyond(const std::string& target_name)
{
    return(IsNaviTarget(target_name) && target_name >= "gfx1030");
}

bool KcUtils::IsVegaTarget(const std::string& target_name)
{
    // Token to identify Vega targets.
    static const char* kVegaTargetToken = "gfx9";
    return (target_name.find(kVegaTargetToken) != std::string::npos);
}

bool KcUtils::IsPostPorcessingSupported(const std::string& isa_file_path)
{
    std::string contents;
    bool ret = KcUtils::ReadTextFile(isa_file_path, contents, nullptr);
    if (ret)
    {
        // If we manage to find a "basic block" symbol, it means
        // that we cannot post-process the disassembly at the moment.
        size_t bbLocation = contents.find("_L1:");
        ret = (bbLocation == std::string::npos);
    }
    return ret;
}

bool KcUtils::IsLlpcDisassembly(const std::string& isa_file_path)
{
    std::string contents;
    bool ret = KcUtils::ReadTextFile(isa_file_path, contents, nullptr);
    if (ret)
    {
        const char* kLlpcToken = "_amdgpu_";
        size_t llpc_token = contents.find(kLlpcToken);
        ret = (llpc_token != std::string::npos);
    }
    return ret;
}

std::string KcUtils::AdjustBaseFileNameBinary(const std::string& user_input_filename, const std::string& device)
{
    static const char* kStrDefaultExtensionBinary = "binary";
    return AdjustBaseFileName(user_input_filename, device, kStrDefaultExtensionBinary);
}

std::string KcUtils::AdjustBaseFileNameIsaDisassembly(const std::string& user_input_filename, const std::string& device)
{
    return AdjustBaseFileName(user_input_filename, device, kStrDefaultFilenameIsa);
}

std::string KcUtils::AdjustBaseFileNameIlDisassembly(const std::string& user_input_filename, const std::string& device)
{
    return AdjustBaseFileName(user_input_filename, device, kStrDefaultFilenameIl);
}

std::string KcUtils::AdjustBaseFileNameStats(const std::string& user_input_filename, const std::string& device)
{
    return AdjustBaseFileName(user_input_filename, device, kStrDefaultExtensionStats);
}

std::string KcUtils::AdjustBaseFileNameLivereg(const std::string& user_input_filename, const std::string& device)
{
    return AdjustBaseFileName(user_input_filename, device, kStrDefaultExtensionLivereg);
}

std::string KcUtils::AdjustBaseFileNameStallAnalysis(const std::string& user_input_filename, const std::string& device)
{
    return AdjustBaseFileName(user_input_filename, device, kStrDefaultExtensionStalls);
}

std::string KcUtils::AdjustBaseFileNameCfg(const std::string& user_input_filename, const std::string& device)
{
    return AdjustBaseFileName(user_input_filename, device, KC_STR_DEFAULT_CFG_SUFFIX);
}
