//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <algorithm>
#include <set>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "AMDTBaseTools/Include/gtString.h"
#include "AMDTOSWrappers/Include/osFilePath.h"
#include "AMDTOSWrappers/Include/osDirectory.h"
#include "AMDTBaseTools/Include/gtAssert.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_cl.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_lightning.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_string_constants.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_utils.h"

// Version information.
#include "VersionInfo/VersionInfo.h"

// Backend.
#include "DeviceInfoUtils.h"
#include "RadeonGPUAnalyzerBackend/Src/be_utils.h"
#include "RadeonGPUAnalyzerBackend/Src/be_program_builder_opencl.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************


// Errors.
static const char* kStrErrorCannotDisaassembleIsa = "Error: failed to disassemble binary output and produce textual ISA code for device: ";
static const char* kStrErrorIsaRequired = "Error: --isa is required for post processing (live register analysis and cfg generation).";
static const char* kStrErrorCannotDisaassembleIl = "Error: failed to disassemble binary output and produce textual IL code for device: ";
static const char* kStrErrorNoKernelsForAnalysis = "Error: no kernels provided for analysis.";

// Warnings.
static const char* kStrWarningLiveregNotSupportedForIsaA = "Warning: live register analysis cannot be performed on the generated ISA for ";
static const char* kStrWarningLiveregNotSupportedForIsaB = " - skipping.";
static const char* kStrWarningCfgNotSupportedForIsaA = "Warning: cfg cannot be generated from the ISA for ";
static const char* kStrWarningCfgNotSupportedForIsaB = " -skipping.";
static const char* kStrWarningIlNotSupportedForRdnaA = "Warning: IL disassembly extraction not supported for RDNA target ";
static const char* kStrWarningIlNotSupportedForRdnaB = " - skipping.";
static const char*  kStrStatsNa = "n/a";

// In case of unsupported targets: list them over here.
static const std::set<std::string>  kUnsupportedTargets = {};

static bool IsInputValid(const Config& config)
{
    bool ret = true;

    // ISA is required for post-processing.
    if (config.isa_file.empty() && (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty()
        || !config.livereg_analysis_file.empty()))
    {
        std::cout << kStrErrorIsaRequired << std::endl;
        ret = false;
    }

    return ret;
}

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

KcCliCommanderCL::KcCliCommanderCL() : is_all_kernels_(false)
{
}

KcCliCommanderCL::~KcCliCommanderCL()
{
    // No need to call DeleteInstance. The base class singleton performs this.
}

// Obsolete: this routine is being used in the generation of the statistics CSV file.
// It should be removed after this mechanism is refactored.
template<class T> std::string doNAFormat(T ui, T sentinal, T err, char list_separator, bool should_add_separator /*= true*/)
{
    std::stringstream s;
    if (ui == sentinal)
    {
        s << "n/a";

        if (should_add_separator)
        {
            s << list_separator;
        }
    }
    else if (ui == err)
    {
        s << "err";

        if (should_add_separator)
        {
            s << list_separator;
        }
    }
    else
    {
        s << ui;

        if (should_add_separator)
        {
            s << list_separator;
        }
    }
    return s.str();
}

template<class T> static string doNAFormat(T ui, T sentinal, T err, char list_separator)
{
    std::stringstream s;
    if (ui == sentinal)
    {
        s << kStrStatsNa << list_separator;
    }
    else if (ui == err)
    {
        s << "err" << list_separator;
    }
    else
    {
        s << ui << list_separator;
    }
    return s.str();
}

bool KcCliCommanderCL::Init(const Config& config, LoggingCallbackFunction callback)
{
    GT_UNREFERENCED_PARAMETER(config);
    log_callback_ = callback;

    // Initialize the backend.
    be_ = Backend::Instance();
    beKA::beStatus be_rc = be_->Initialize(kProgramTypeOpencl, callback);
    bool ret = (be_rc == beKA::kBeStatusSuccess);

    if (ret)
    {
        // Initialize the devices list.
        std::set<std::string>  devices;
        be_rc = be_->theOpenCLBuilder()->GetDevices(devices);
        ret = (be_rc == beKA::kBeStatusSuccess);

        // Apply any filters to the targets here.
        std::set<std::string> filtered_targets;
        for (const std::string& target : devices)
        {
            filtered_targets.insert(target);
        }

        // Only external (non-placeholder) and based on CXL version devices should be used.
        if (ret)
        {
            beKA::beStatus rc_inner = be_->theOpenCLBuilder()->GetDeviceTable(table_);
            ret = (rc_inner == beKA::kBeStatusSuccess);
            if (ret)
            {
                for (vector<GDT_GfxCardInfo>::const_iterator it = table_.begin(); it != table_.end(); ++it)
                {
                    if (kUnsupportedTargets.count(it->m_szCALName) == 0 &&
                        (filtered_targets.find(it->m_szCALName) != filtered_targets.end()))
                    {
                        external_devices_.insert(it->m_szCALName);
                    }
                }
            }
        }
    }
    return ret;
}

bool KcCliCommanderCL::Compile(const Config& config)
{
    bool ret = IsInputValid(config);
    if (ret)
    {
        // Verify that an input file was specified
        if (config.input_files.size() != 1 || config.input_files[0].empty())
        {
            std::stringstream log_stream;
            log_stream << kStrErrorSingleInputFileExpected << std::endl;
            LogCallback(log_stream.str());
        }
        else
        {
            std::string source_code;
            ret = KcUtils::ReadProgramSource(config.input_files[0], source_code);

            if (!ret)
            {
                std::stringstream log_stream;
                log_stream << kStrErrorCannotReadFile << config.input_files[0] << endl;
                LogCallback(log_stream.str());
            }
            else
            {
                OpenCLOptions options;
                options.mode = kModeOpencl;
                options.selected_devices_sorted = asics_sorted_;
                options.defines = config.defines;
                options.opencl_compile_options = config.opencl_options;

                int num_successfull_builds = 0;
                beKA::beStatus beRet = kBeStatusInvalid;
                if (config.include_path.size() > 0)
                {
                    beRet = be_->theOpenCLBuilder()->Compile(source_code, options, config.input_files[0], &config.include_path, num_successfull_builds);
                }
                else
                {
                    beRet = be_->theOpenCLBuilder()->Compile(source_code, options, config.input_files[0], NULL, num_successfull_builds);
                }

                if (beRet == beKA::kBeStatusSuccess)
                {
                    ret = true;
                }
                else
                {
                    ret = false;
                }
            }
        }
    }

    return ret;
}

bool KcCliCommanderCL::ListEntries(const Config& config, LoggingCallbackFunction callback)
{
    return KcCLICommanderLightning::ListEntriesRocmCL(config, callback);
}

void KcCliCommanderCL::RunCompileCommands(const Config& config, LoggingCallbackFunction callback)
{
    if (Init(config, callback))
    {
        if (InitRequestedAsicList(config.asics, config.mode, external_devices_, asics_, false))
        {

            // Sort the targets.
            for (const std::string& target : asics_)
            {
                asics_sorted_.push_back(target);
            }
            std::sort(asics_sorted_.begin(), asics_sorted_.end(), &BeUtils::DeviceNameLessThan);

            if (Compile(config))
            {
                InitRequiredKernels(config, asics_sorted_, required_kernels_);

                GetBinary(config);

                GetISAText(config);

                GetILText(config);

                Analysis(config);

                GetMetadata(config);
            }
        }
    }
}

bool KcCliCommanderCL::PrintAsicList(const Config&)
{
    // We do not want to display names that contain these strings.
    const char* kFilterIndicator1 = ":";
    const char* kFilterIndicator2 = "Not Used";

    bool  result = false;
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
            std::cout << pair.first << std::endl;
            for (const std::string& card : pair.second)
            {
                // Filter out internal names.
                if (card.find(kFilterIndicator1) == std::string::npos &&
                    card.find(kFilterIndicator2) == std::string::npos)
                {
                    std::cout << "\t" << card << std::endl;
                }
            }
        }
        result = true;
    }

    return result;
}

void KcCliCommanderCL::Analysis(const Config& config)
{
    if (!config.analysis_file.empty())
    {
        for (const std::string& required_target : asics_sorted_)
        {
            if (!be_->theOpenCLBuilder()->HasCodeObjectBinary(required_target))
            {
                if (required_kernels_.size() == 0)
                {
                    std::stringstream s_Log;
                    s_Log << kStrErrorNoKernelsForAnalysis << endl;
                    LogCallback(s_Log.str());
                }

                if ((config.suppress_section.size() > 0) && (config.binary_output_file.size() == 0))
                {
                    std::stringstream s_Log;
                    s_Log << kStrWarningOpenclSupressWithoutBinary << std::endl;
                    LogCallback(s_Log.str());
                }

                for (const std::string& kernel_name : required_kernels_)
                {
                    // Show the analysis only for external devices.
                    if (external_devices_.find(required_target) == external_devices_.end())
                    {
                        continue;
                    }

                    AnalysisData analysis;
                    (void)memset(&analysis, 0, sizeof(analysis));
                    beStatus status = be_->theOpenCLBuilder()->GetStatistics(required_target, kernel_name, analysis);
                    if (status != kBeStatusSuccess)
                    {
                        if (status == kBeStatusWrongKernelName)
                        {
                            std::stringstream log;
                            log << "Info: Skipping analysis, wrong kernel name provided: '" << config.function << "'." << endl;
                            LogCallback(log.str());
                        }

                        continue;
                    }

                    // Write the stats file.
                    WriteAnalysisFile(config, kernel_name, required_target, analysis);
                }
            }
            else
            {
                // Handle the CodeObject case.
                std::map<std::string, beKA::AnalysisData> stats_map;
                bool is_stats_extracted = be_->theOpenCLBuilder()->ExtractStatisticsCodeObject(required_target, stats_map);
                assert(is_stats_extracted);
                if (is_stats_extracted)
                {
                    for (auto iter = stats_map.begin(); iter != stats_map.end(); iter++)
                    {
                        // Write the stats file.
                        WriteAnalysisFile(config, iter->first, required_target, iter->second);
                    }
                }
            }
        }
    }
}

void KcCliCommanderCL::GetILText(const Config& config)
{
    if (!config.il_file.empty())
    {
        if (!config.il_file.empty() && !config.function.empty() && be_ != nullptr)
        {
            BeProgramBuilderOpencl* builder = be_->theOpenCLBuilder();
            if (builder != nullptr)
            {
                if ((config.suppress_section.size() > 0) && (config.binary_output_file.size() == 0))
                {
                    // Print the warning message.
                    std::stringstream msg;
                    msg << kStrWarningOpenclSupressWithoutBinary << std::endl;
                    LogCallback(msg.str());
                }

                // This variable will hold the IL text.
                std::string il_text_buffer;

                // Get IL text and make output files.
                for (const std::string& device_name : asics_)
                {
                    if (!KcUtils::IsNaviTarget(device_name))
                    {
                        for (const std::string& kernel_name : required_kernels_)
                        {
                            beKA::beStatus status = builder->GetKernelIlText(device_name, kernel_name, il_text_buffer);
                            if (status == kBeStatusSuccess)
                            {
                                gtString il_output_filename;
                                KcUtils::ConstructOutputFileName(config.il_file, "", kStrDefaultExtensionAmdil,
                                    kernel_name, device_name, il_output_filename);
                                KcUtils::WriteTextFile(il_output_filename.asASCIICharArray(), il_text_buffer, log_callback_);
                            }
                            else
                            {
                                // Inform the user.
                                std::stringstream msg;
                                msg << kStrErrorCannotDisaassembleIl << device_name;

                                // If we have the kernel name - specify it in the error message.
                                if (!kernel_name.empty())
                                {
                                    msg << " (kernel: " << kernel_name << ")";
                                }

                                // Print the message.
                                msg << "." << std::endl;
                                log_callback_(msg.str().c_str());
                            }

                            // Clear the output buffer.
                            il_text_buffer.clear();
                        }
                    }
                    else
                    {
                        std::cout << kStrWarningIlNotSupportedForRdnaA <<
                            device_name << kStrWarningIlNotSupportedForRdnaB << std::endl;
                    }
                }
            }
        }
    }
}

void KcCliCommanderCL::GetISAText(const Config& config)
{
    if (!config.isa_file.empty() || !config.analysis_file.empty() ||
        !config.block_cfg_file.empty() || !config.inst_cfg_file.empty() || !config.livereg_analysis_file.empty())
    {
        if ((config.suppress_section.size() > 0) && (config.binary_output_file.size() == 0))
        {
            // Print the warning message.
            std::stringstream msg;
            msg << kStrWarningOpenclSupressWithoutBinary << std::endl;
            LogCallback(msg.str());
        }

        BeProgramBuilderOpencl* cl_builder = be_->theOpenCLBuilder();
        if (cl_builder != nullptr)
        {
            std::string isa_il;
            bool should_create_sub_directories = is_all_kernels_ && !required_kernels_.empty();
            GT_UNREFERENCED_PARAMETER(should_create_sub_directories);

            // Perform live register analysis - blocked until analysis engine is improved to support
            // OpenCL disassembly with better stability.
            bool is_post_processing_enabled = true;
            bool is_livereg_required = !config.livereg_analysis_file.empty();
            bool is_cfg_required = !config.block_cfg_file.empty() || !config.inst_cfg_file.empty();

            std::map<std::string, std::string> device_to_code_object_disassembly_mapping;
            cl_builder->GetDeviceToCodeObjectDisassemblyMapping(device_to_code_object_disassembly_mapping);
            if (!config.isa_file.empty() || !config.il_file.empty())
            {
                // Get ISA text and make output files.
                for (const std::string& device_name : asics_)
                {
                    auto iter = device_to_code_object_disassembly_mapping.find(device_name);
                    if (iter != device_to_code_object_disassembly_mapping.end())
                    {
                        // Write the ISA disassembly text file.
                        gtString isa_output_filename;
                        KcUtils::ConstructOutputFileName(config.isa_file, "", kStrDefaultExtensionIsa, "", device_name, isa_output_filename);
                        KcUtils::WriteTextFile(isa_output_filename.asASCIICharArray(), iter->second, log_callback_);

                        // Perform post processing.
                        is_post_processing_enabled = KcUtils::IsPostPorcessingSupported(isa_output_filename.asASCIICharArray());
                        if (is_livereg_required)
                        {
                            if (is_post_processing_enabled)
                            {
                                gtString liveRegAnalysisOutputFileName;
                                KcUtils::ConstructOutputFileName(config.livereg_analysis_file, kStrDefaultExtensionLivereg,
                                    kStrDefaultExtensionText, "", device_name, liveRegAnalysisOutputFileName);

                                // Call the kcUtils routine to analyze <generatedFileName> and write the analysis file.
                                KcUtils::PerformLiveRegisterAnalysis(isa_output_filename, liveRegAnalysisOutputFileName,
                                    log_callback_, config.print_process_cmd_line);
                            }
                            else
                            {
                                std::cout << kStrWarningLiveregNotSupported << "for " <<
                                    device_name << " " << kStrWarningSkipping << std::endl;
                            }
                        }

                        if (is_cfg_required)
                        {
                            bool is_per_block = !config.block_cfg_file.empty();
                            std::cout << (is_per_block ? kStrInfoContructingPerBlockCfg1 : kStrInfoContructingPerInstructionCfg1) << device_name << "..." << std::endl;
                            if (is_post_processing_enabled)
                            {
                                if (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty())
                                {
                                    gtString cfg_output_filename;
                                    std::string base_name = (!config.block_cfg_file.empty() ? config.block_cfg_file : config.inst_cfg_file);
                                    KcUtils::ConstructOutputFileName(base_name, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot,
                                        "", device_name, cfg_output_filename);

                                    // Call the kcUtils routine to analyze <generatedFileName> and write
                                    // the analysis file.
                                    KcUtils::GenerateControlFlowGraph(isa_output_filename, cfg_output_filename, log_callback_,
                                        !config.inst_cfg_file.empty(), config.print_process_cmd_line);
                                }
                            }
                            else
                            {
                                std::cout << kStrWarningCfgNotSupported << "for " << device_name << " " << kStrWarningSkipping << std::endl;
                            }
                        }
                    }
                    else
                    {
                        bool is_isa_file_temp = config.isa_file.empty();
                        for (const std::string& kernel_name : required_kernels_)
                        {
                            beKA::beStatus status = cl_builder->GetKernelIsaText(device_name, kernel_name, isa_il);
                            if (status == kBeStatusSuccess)
                            {
                                gtString isa_output_filename;
                                if (is_isa_file_temp)
                                {
                                    gtString  isa_filename, isa_file_extension;
                                    isa_filename << (std::string(kStrDefaultFilenameIsa) + device_name + kernel_name).c_str();
                                    isa_file_extension << kStrDefaultExtensionIsa;
                                    isa_output_filename = KcUtils::ConstructTempFileName(isa_filename, isa_file_extension);
                                }
                                else
                                {
                                    KcUtils::ConstructOutputFileName(config.isa_file, "", kStrDefaultExtensionIsa, kernel_name, device_name, isa_output_filename);
                                }
                                KcUtils::WriteTextFile(isa_output_filename.asASCIICharArray(), isa_il, log_callback_);

                                // Perform live register analysis.
                                is_post_processing_enabled = KcUtils::IsPostPorcessingSupported(isa_output_filename.asASCIICharArray());
                                if (is_post_processing_enabled)
                                {
                                    if (is_livereg_required)
                                    {
                                        std::cout << kStrInfoPerformingLiveregAnalysis1 <<
                                            "kernel " << kernel_name << " (" << device_name << ")..." << std::endl;

                                        gtString livereg_analysis_output_filename;
                                        KcUtils::ConstructOutputFileName(config.livereg_analysis_file, kStrDefaultExtensionLivereg,
                                            kStrDefaultExtensionText, kernel_name, device_name, livereg_analysis_output_filename);

                                        // Call the kcUtils routine to analyze <generatedFileName> and write the analysis file.
                                        bool is_livereg_performed = KcUtils::PerformLiveRegisterAnalysis(isa_output_filename, livereg_analysis_output_filename,
                                            log_callback_, config.print_process_cmd_line);

                                        if (is_livereg_performed)
                                        {
                                            if (KcUtils::FileNotEmpty(livereg_analysis_output_filename.asASCIICharArray()))
                                            {
                                                std::cout << kStrInfoSuccess << std::endl;
                                            }
                                            else
                                            {
                                                std::cout << kStrInfoFailed << std::endl;
                                            }
                                        }
                                    }
                                }
                                else if(is_livereg_required)
                                {
                                    std::cout << kStrWarningLiveregNotSupported << "for " <<
                                        device_name << " " << kStrWarningSkipping << std::endl;
                                }

                                // Generate control flow graph.
                                if (is_post_processing_enabled)
                                {
                                    if (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty())
                                    {
                                        bool is_per_block = !config.block_cfg_file.empty();
                                        std::cout << (is_per_block ? kStrInfoContructingPerBlockCfg1 : kStrInfoContructingPerInstructionCfg1) <<
                                            "kernel " << kernel_name << " (" << device_name << ")..." << std::endl;

                                        gtString cfg_output_filename;
                                        std::string base_name = (!config.block_cfg_file.empty() ? config.block_cfg_file : config.inst_cfg_file);
                                        KcUtils::ConstructOutputFileName(base_name, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot,
                                            kernel_name, device_name, cfg_output_filename);

                                        // Call the kcUtils routine to analyze <generatedFileName> and write
                                        // the analysis file.
                                        bool is_cfg_generated =  KcUtils::GenerateControlFlowGraph(isa_output_filename, cfg_output_filename, log_callback_,
                                            !config.inst_cfg_file.empty(), config.print_process_cmd_line);

                                        if (is_cfg_generated)
                                        {
                                            if (KcUtils::FileNotEmpty(cfg_output_filename.asASCIICharArray()))
                                            {
                                                std::cout << kStrInfoSuccess << std::endl;
                                            }
                                            else
                                            {
                                                std::cout << kStrInfoFailed << std::endl;
                                            }
                                        }
                                    }
                                }
                                else if (is_cfg_required)
                                {
                                    std::cout << kStrWarningCfgNotSupported << "for " <<
                                        device_name << " " << kStrWarningSkipping << std::endl;
                                }

                                // Delete temporary files.
                                if (config.isa_file.empty())
                                {
                                    if (KcUtils::FileNotEmpty(isa_output_filename.asASCIICharArray()))
                                    {
                                        KcUtils::DeleteFile(isa_output_filename);
                                    }
                                }
                            }
                            else
                            {
                                // Inform the user.
                                std::stringstream msg;
                                msg << kStrErrorCannotDisaassembleIsa << device_name;

                                // If we have the kernel name - specify it in the error message.
                                if (!kernel_name.empty())
                                {
                                    msg << " (kernel: " << kernel_name << ")";
                                }

                                // Print the message.
                                msg << "." << std::endl;
                                log_callback_(msg.str().c_str());
                            }

                            // Clear the output buffer.
                            isa_il.clear();
                        }
                    }
                }
            }
        }
    }
}

void KcCliCommanderCL::GetBinary(const Config& config)
{
    if (config.binary_output_file.size() > 0 && be_ != nullptr)
    {
        BeProgramBuilderOpencl* builder = be_->theOpenCLBuilder();
        if (builder != nullptr)
        {
            // Create binary output files.
            BinaryOptions bin_opts;
            bin_opts.suppress_section = config.suppress_section;

            std::vector<char> binary;
            for (const std::string& device_name : asics_)
            {
                beStatus status = builder->GetBinary(device_name, bin_opts, binary);
                if (status == kBeStatusSuccess)
                {
                    gtString bin_output_filename;
                    KcUtils::ConstructOutputFileName(config.binary_output_file, "", kStrDefaultExtensionBin,
                                                     "", device_name, bin_output_filename);
                    KcUtils::WriteBinaryFile(bin_output_filename.asASCIICharArray(), binary, log_callback_);
                }
                else
                {
                    // Inform the user.
                    std::stringstream msg;
                    msg << kStrErrorCannotExtractBinaries << " for " << device_name << "." << std::endl;
                    log_callback_(msg.str().c_str());
                }

                // Clear the output buffer.
                binary.clear();
            }
        }
    }
}

void KcCliCommanderCL::GetMetadata(const Config& config)
{
    if (config.metadata_file.size() > 0 && be_ != nullptr)
    {
        if ((config.suppress_section.size() > 0) && (config.binary_output_file.size() == 0))
        {
            // Print the warning message.
            std::stringstream msg;
            msg << kStrWarningOpenclSupressWithoutBinary << std::endl;
            LogCallback(msg.str());
        }

        BeProgramBuilderOpencl* builder = be_->theOpenCLBuilder();
        if (builder != nullptr)
        {
            // Create the meta-data output files.
            std::string metadata_text;
            for (const std::string& device_name : asics_)
            {
                for (const std::string& kernel_name : required_kernels_)
                {
                    beStatus status = builder->GetKernelMetaDataText(device_name, kernel_name, metadata_text);
                    if (status == kBeStatusSuccess)
                    {
                        gtString metadata_output_filename;
                        KcUtils::ConstructOutputFileName(config.metadata_file, "", kStrDefaultExtensionMetadata,
                                                         kernel_name, device_name, metadata_output_filename);
                        KcUtils::WriteTextFile(metadata_output_filename.asASCIICharArray(), metadata_text, log_callback_);
                    }
                    else if (status == kBeStatusNoMetadataForDevice)
                    {
                        log_callback_(std::string(kStrWarningOpenclMetadataNotSupported1) + device_name + kStrWarningOpenclMetadataNotSupported2 + "\n");
                        break;
                    }
                    else
                    {
                        // Inform the user.
                        std::stringstream msg;
                        msg << kStrErrorCannotExtractMetadata << " for " << device_name << "(kernel: " << kernel_name << ")." << std::endl;
                        log_callback_(msg.str().c_str());
                    }

                    // Clear the output buffer.
                    metadata_text.clear();
                }
            }
        }
    }
}

void KcCliCommanderCL::InitRequiredKernels(const Config& config, const std::vector<std::string>& required_devices, std::vector<std::string>& required_kernels)
{
    required_kernels.clear();
    if (!required_devices.empty())
    {
        // We only need a single device name.
        auto first_device = required_devices.begin();
        const std::string& device_name = *first_device;
        std::string requested_kernel = config.function;
        std::transform(requested_kernel.begin(), requested_kernel.end(), requested_kernel.begin(), ::tolower);
        // Process all kernels by default or if all kernels are explicitly requested.
        is_all_kernels_ = (requested_kernel.compare("all") == 0 || requested_kernel.empty());
        BeProgramBuilderOpencl* cl_builder = be_->theOpenCLBuilder();
        if (cl_builder != nullptr)
        {
            if (is_all_kernels_)
            {
                cl_builder->GetKernels(device_name, required_kernels);
            }
            else
            {
                required_kernels.push_back(config.function);
            }
        }
    }
}

void KcCliCommanderCL::WriteAnalysisFile(const Config& config, const std::string& kernel_name,
    const std::string& device_name, const beKA::AnalysisData& analysis)
{
    // Create the output file.
    ofstream output;
    gtString stats_output_filename;
    KcUtils::ConstructOutputFileName(config.analysis_file, kStrDefaultExtensionStats,
        kStrDefaultExtensionCsv, kernel_name, device_name, stats_output_filename);
    osFilePath analysis_output_path;
    analysis_output_path.setFullPathFromString(stats_output_filename);
    osDirectory target_dir;
    analysis_output_path.getFileDirectory(target_dir);
    gtString targetDirAsStr = target_dir.directoryPath().asString(true);
    analysis_output_path.setFileDirectory(targetDirAsStr);

    // Create the target directory if it does not exist.
    if (!target_dir.IsEmpty() && !target_dir.exists())
    {
        bool is_target_dir_created(target_dir.create());

        if (!is_target_dir_created)
        {
            std::stringstream error_msg;
            error_msg << kStrErrorCannotFindOutputDir << std::endl;
            LogCallback(error_msg.str());
        }
    }

    // Get the separator for CSV list items.
    char csv_separator = KcUtils::GetCsvSeparator(config);

    // Generate the content for the file.
    std::stringstream file_content;

    // Headers.
    file_content << KcUtils::GetStatisticsCsvHeaderString(csv_separator) << std::endl;

    // CSV line.
    file_content << device_name << csv_separator;
    file_content << analysis.scratch_memory_used << csv_separator;
    file_content << analysis.num_threads_per_group_total << csv_separator;

    // For Navi targets, the wave size is determined in runtime.
    bool is_navi_target = KcUtils::IsNaviTarget(device_name);
    file_content << (!is_navi_target ? doNAFormat(analysis.wavefront_size,
        kCalValue64Na, kCalValue64Error, csv_separator) : kStrStatsNa);
    if (is_navi_target)
    {
        file_content << csv_separator;
    }

    file_content << analysis.lds_size_available << csv_separator;
    file_content << analysis.lds_size_used << csv_separator;
    file_content << doNAFormat(analysis.num_sgprs_available, kCalValue64Na, kCalValue64Error, csv_separator);
    file_content << doNAFormat(analysis.num_sgprs_used, kCalValue64Na, kCalValue64Error, csv_separator);
    file_content << doNAFormat(analysis.num_sgpr_spills, kCalValue64Na, kCalValue64Error, csv_separator);
    file_content << doNAFormat(analysis.num_vgprs_available, kCalValue64Na, kCalValue64Error, csv_separator);
    file_content << doNAFormat(analysis.num_vgprs_used, kCalValue64Na, kCalValue64Error, csv_separator);
    file_content << doNAFormat(analysis.num_vgpr_spills, kCalValue64Na, kCalValue64Error, csv_separator);
    file_content << doNAFormat(analysis.num_threads_per_group_x, kCalValue64Na, kCalValue64Error, csv_separator);
    file_content << doNAFormat(analysis.num_threads_per_group_y, kCalValue64Na, kCalValue64Error, csv_separator);
    file_content << doNAFormat(analysis.num_threads_per_group_z, kCalValue64Na, kCalValue64Error, csv_separator);
    file_content << doNAFormat(analysis.isa_size, (CALuint64)0, kCalValue64Error, csv_separator, false);
    file_content << std::endl;

    // Write the file.
    output.open(analysis_output_path.asString().asASCIICharArray());
    if (!output.is_open())
    {
        std::stringstream log;
        log << "Error: Unable to open " << config.analysis_file << " for write.\n";
        LogCallback(log.str());
    }
    else
    {
        // Write the contents.
        output << file_content.str();
        output.close();
    }
}
