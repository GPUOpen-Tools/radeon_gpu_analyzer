//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifdef _WIN32
// Infra.
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4309)
#endif
#include "AMDTBaseTools/Include/gtString.h"
#include "AMDTBaseTools/Include/gtAssert.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

// Backend.
#include "RadeonGPUAnalyzerBackend/Src/be_program_builder_dx11.h"
#include "RadeonGPUAnalyzerBackend/Src/be_utils.h"
#include "RadeonGPUAnalyzerBackend/Src/be_static_isa_analyzer.h"
#include "CElf.h"
#include "DeviceInfoUtils.h"

// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_dx11.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_string_constants.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_utils.h"

// Constants.
static const int  kDxMaxSupportedShaderModelMajor = 5;
static const int  kDxMaxSupportedShaderModelMinor = 0;

// Error strings.
static const std::string kStrDx11NaValue = "N/A";
static const char* kStrErrorShaderModelNotSupported = "Error: shader model 5.1 and above is not supported in DX11 mode. Please use DX12 mode (rga -s dx12 -h).";
static const char* kStrErrorDx11CannotListAdapters = "Error: failed to get the list of display adapters installed on this system.";
static const char* kStrErrorDx11AdapterSetFailed = "Error: failed to set display adapter with provided ID.";
static const char* kStrErrorDx11IncorrectShaderModel = "Error: incorrect DX shader model provided.";
static const char* kStrErrorDx11UnsupportedShaderModel1 = "Error: unsupported Shader Model detected: ";
static const char* kStrErrorDx11UnsupportedShaderModel2 = "RGA supports Shader Model ";
static const char* kStrErrorDx11UnsupportedShaderModel3 = "and below.";

// Warning strings.
static const char* kStrWarningDx11ShaderModelIgnored = "Warning: shader model (-p / --profile option) is ignored since the input shaders are not in HLSL. Shader model is only used for front-end compilation of HLSL.";

// Info strings.
static const char* kStrInfoDx11FoundAdapters = "Info: found the following supported display adapters installed on this system:";
static const char* kStrInfoDx11ExtractingBinary = "Extracting Binary for ";
static const char* kStrInfoDx11ExtractingAmdil = "Extracting AMD IL code for ";
static const char* kStrInfoDx11DxAsmCodeGenerationSuccess = "DX ASM code generation succeeded.";
static const char* kStrInfoDx11DxAsmCodeGenerationFailure = "DX ASM code generation failed.";

KcCliCommanderDX::KcCliCommanderDX(void)
{
    backend_handler_ = nullptr;
}

KcCliCommanderDX::~KcCliCommanderDX(void)
{
    // No need to call DeleteInstance. The base class singleton performs this.
}

void KcCliCommanderDX::ListAdapters(Config & config, LoggingCallbackFunction callback)
{
    std::vector<std::string> adapter_names;
    stringstream msg;

    if (BeProgramBuilderDx11::GetSupportedDisplayAdapterNames(config.print_process_cmd_line, adapter_names))
    {
        msg << kStrInfoDx11FoundAdapters << std::endl << std::endl;
        for (size_t i = 0; i < adapter_names.size(); i++)
        {
            msg << " " << i << "\t" << adapter_names[i] << std::endl;
        }
    }
    else
    {
        msg << kStrErrorDx11CannotListAdapters << std::endl;
    }

    msg << std::endl;
    callback(msg.str());
}

void KcCliCommanderDX::InitRequestedAsicListDX(const Config& config)
{
    stringstream log;

    // Get the default device list.
    if (!config.asics.empty())
    {
        dx_default_asics_list_.clear();
        std::vector<GDT_GfxCardInfo> dx_device_table;
        std::set<std::string> supported_devices;
        std::set<std::string> matched_targets;

        if (BeUtils::GetAllGraphicsCards(dx_device_table, supported_devices))
        {
            if (InitRequestedAsicList(config.asics, config.mode, supported_devices, matched_targets, false))
            {
                for (const std::string& target : matched_targets)
                {
                    for (const GDT_GfxCardInfo& dxDevice : dx_device_table)
                    {
                        if (dxDevice.m_szCALName == target)
                        {
                            dx_default_asics_list_.push_back(dxDevice);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void KcCliCommanderDX::ExtractISA(const string& device_name, const Config& config, size_t& isa_size_in_bytes,
    string isa_buffer, bool& is_isa_size_detected, bool& should_detect_isa_size)
{
    BeProgramBuilderDx11* program_builder_dx = backend_handler_ != nullptr ? backend_handler_->theOpenDXBuilder() : nullptr;
    beStatus backend_rc = kBeStatusInvalid;
    GT_IF_WITH_ASSERT(program_builder_dx != nullptr)
    {
        backend_rc = program_builder_dx->GetDxShaderIsaText(device_name, isa_buffer);
        string filename = config.isa_file;

        if (backend_rc == kBeStatusSuccess)
        {
            gtString isa_output_filename;
            if (filename.empty())
            {
                gtString tempIsaFileName, isaFileExt;
                tempIsaFileName << (std::string(kStrDefaultFilenameIsa) + device_name + config.function).c_str();
                isaFileExt << kStrDefaultExtensionIsa;
                isa_output_filename = KcUtils::ConstructTempFileName(tempIsaFileName, isaFileExt);
            }
            else
            {
                KcUtils::ConstructOutputFileName(config.isa_file, "", kStrDefaultExtensionIsa,
                    config.function, device_name, isa_output_filename);
            }
            KcUtils::WriteTextFile(isa_output_filename.asASCIICharArray(), isa_buffer, log_callback_);

            // Save parsed ISA to a CSV file if it's requested
            if (config.is_parsed_isa_required)
            {
                std::string  parsedIsa, parsedIsaFileName;
                backend_rc = BeProgramBuilder::ParseIsaToCsv(isa_buffer, device_name, parsedIsa);

                if (backend_rc == beKA::kBeStatusSuccess)
                {
                    backend_rc = KcUtils::GetParsedISAFileName(isa_output_filename.asASCIICharArray(), parsedIsaFileName) ?
                        backend_rc : beKA::kBeStatusLightningConstructISAFileNameFailed;
                }
                if (backend_rc == beKA::kBeStatusSuccess)
                {
                    KcUtils::WriteTextFile(parsedIsaFileName, parsedIsa, log_callback_);
                }
            }

            // Detect the ISA size.
            is_isa_size_detected = program_builder_dx->GetIsaSize(isa_buffer, isa_size_in_bytes);

            // If we managed to detect the ISA size, don't do it again.
            should_detect_isa_size = !is_isa_size_detected;

            // We need to pre-process the ISA in case that any post-processing
            // operation is going to be performed.
            bool should_pre_process_isa = !config.livereg_analysis_file.empty() ||
                !config.inst_cfg_file.empty() || !config.block_cfg_file.empty();

            std::string pre_processed_isa_file;
            if (should_pre_process_isa)
            {
                // Generate a pre-processed ISA file before passing the ISA for analysis.
                // This is required since, sometimes, depending on the order of compilation,
                // the ISA disassembly for even pre-Navi targets would be in the Navi format.
                pre_processed_isa_file = KcUtils::ConstructTempFileName(kStrDefaultFilenamePreprocessedIsa, "txt");
                assert(!pre_processed_isa_file.empty());

                if (config.print_process_cmd_line)
                {
                    std::cout << "Creating temporary file for pre-processed ISA: " << pre_processed_isa_file << std::endl;
                }

                // Pre-process the ISA.
                BeStaticIsaAnalyzer::PreprocessIsaFile(isa_output_filename.asASCIICharArray(), pre_processed_isa_file);
            }

            if (!config.livereg_analysis_file.empty())
            {
                // Perform the live register analysis.
                gtString liveRegAnalysisOutputFileName;
                KcUtils::ConstructOutputFileName(config.livereg_analysis_file, kStrDefaultExtensionLivereg,
                    kStrDefaultExtensionText, config.function,
                    device_name, liveRegAnalysisOutputFileName);

                // Call the kcUtils routine to analyze <generatedFileName> and write the analysis file.
                KcUtils::PerformLiveRegisterAnalysis(pre_processed_isa_file, liveRegAnalysisOutputFileName.asASCIICharArray(),
                    log_callback_, config.print_process_cmd_line);
            }

            if (!config.inst_cfg_file.empty() || !config.block_cfg_file.empty())
            {
                gtString cfg_output_ilename;
                std::string base_name = (!config.inst_cfg_file.empty() ? config.inst_cfg_file : config.block_cfg_file);
                KcUtils::ConstructOutputFileName(base_name, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot,
                    config.function, device_name, cfg_output_ilename);

                KcUtils::GenerateControlFlowGraph(pre_processed_isa_file, cfg_output_ilename.asASCIICharArray(), log_callback_,
                    !config.inst_cfg_file.empty(), config.print_process_cmd_line);
            }

            // Delete the temporary pre-processed ISA file.
            if (should_pre_process_isa)
            {
                BeUtils::DeleteFileFromDisk(pre_processed_isa_file);
            }

            // Delete temporary ISA file.
            if (filename.empty())
            {
                KcUtils::DeleteFile(isa_output_filename);
            }
        }

        if (backend_rc == kBeStatusSuccess)
        {
            std::stringstream log;
            log << kStrInfolExtractingIsaForDevice << device_name << "... " << kStrInfoSuccess << std::endl;
            LogCallback(log.str());
        }
        else
        {
            std::stringstream log;
            log << kStrInfolExtractingIsaForDevice << device_name << "... " << kStrInfoFailed << std::endl;
            LogCallback(log.str());
        }
    }
}

void KcCliCommanderDX::ExtractIL(const std::string& device_name, const Config& config)
{
    BeProgramBuilderDx11* program_builder_dx = backend_handler_ != nullptr ? backend_handler_->theOpenDXBuilder() : nullptr;
    beStatus backend_rc = kBeStatusInvalid;
    GT_IF_WITH_ASSERT(program_builder_dx != nullptr)
    {
        std::string il_buffer;
        backend_rc = program_builder_dx->GetDxShaderIl(device_name, il_buffer);

        if (backend_rc == kBeStatusSuccess)
        {
            gtString il_output_filename;
            KcUtils::ConstructOutputFileName(config.il_file, "", kStrDefaultExtensionAmdil,
                config.function, device_name, il_output_filename);
            KcUtils::WriteTextFile(il_output_filename.asASCIICharArray(), il_buffer, log_callback_);
        }

        if (backend_rc == kBeStatusSuccess)
        {
            std::stringstream log;
            log << kStrInfoDx11ExtractingAmdil << device_name << "... " << kStrInfoSuccess << std::endl;
            LogCallback(log.str());
        }
        else
        {
            std::stringstream log;
            log << kStrInfoDx11ExtractingAmdil << device_name << "... " << kStrInfoFailed << std::endl;
            LogCallback(log.str());
        }
    }
}

bool KcCliCommanderDX::ExtractStats(const string& device_name, const Config& config, bool should_detect_isa_size, string isaBuffer, bool is_isa_size_detected,
    size_t isa_size_in_bytes, std::vector<AnalysisData>& analysis_data, std::vector<string>& device_analysis_data)
{
    AnalysisData analysis;
    beStatus backend_rc = backend_handler_->theOpenDXBuilder()->GetStatistics(device_name, config.function, analysis);
    if (backend_rc == kBeStatusSuccess)
    {
        if (should_detect_isa_size)
        {
            backend_rc = backend_handler_->theOpenDXBuilder()->GetDxShaderIsaText(device_name, isaBuffer);
            if (backend_rc == kBeStatusSuccess)
            {
                // Detect the ISA size.
                is_isa_size_detected = backend_handler_->theOpenDXBuilder()->GetIsaSize(isaBuffer, isa_size_in_bytes);
            }
        }

        if (is_isa_size_detected)
        {
            analysis.isa_size = isa_size_in_bytes;
        }
        else
        {
            // Assign largest unsigned value, used as warning
            LogCallback("Warning: ISA size not available.\n");
        }

        // Get the wavefront size.
        size_t wavefront_size = 0;
        if (backend_handler_->theOpenDXBuilder()->GetWavefrontSize(device_name, wavefront_size))
        {
            analysis.wavefront_size = wavefront_size;
        }
        else
        {
            LogCallback("Warning: wavefrontSize size not available.\n");
        }

        analysis_data.push_back(analysis);
        device_analysis_data.push_back(device_name);
        std::stringstream s_Log;
        s_Log << kStrInfoExtractingStats << "... " << kStrInfoSuccess << std::endl;
        LogCallback(s_Log.str());
    }
    else
    {
        std::stringstream s_Log;
        s_Log << kStrInfoExtractingStats << "... " << kStrInfoFailed << std::endl;
        LogCallback(s_Log.str());
    }
    return is_isa_size_detected;
}


void KcCliCommanderDX::ExtractBinary(const std::string& device_name, const Config& config)
{
    std::vector<char> binary;
    beKA::BinaryOptions bin_options;
    bin_options.suppress_section = config.suppress_section;
    if (kBeStatusSuccess == backend_handler_->theOpenDXBuilder()->GetBinary(device_name, bin_options, binary))
    {
        gtString bin_output_filename;
        KcUtils::ConstructOutputFileName(config.binary_output_file, "", kStrDefaultExtensionBin, "", device_name, bin_output_filename);
        KcUtils::WriteBinaryFile(bin_output_filename.asASCIICharArray(), binary, log_callback_);
        std::stringstream log;
        log << kStrInfoDx11ExtractingBinary << device_name << "... " << kStrInfoSuccess << std::endl;
        LogCallback(log.str());
    }
    else
    {
        // Inform the user.
        std::stringstream msg;
        msg << kStrErrorCannotExtractBinaries << " for " << device_name << "." << std::endl;
        log_callback_(msg.str().c_str());
    }
}

/// Output the ISA representative of the compilation
void KcCliCommanderDX::RunCompileCommands(const Config& config, LoggingCallBackFuncP callback)
{
    bool is_init_successful = Init(config, callback);
    if (is_init_successful)
    {
        const bool is_isa_required = !config.isa_file.empty();
        const bool is_il_required = !config.il_file.empty();
        const bool is_statistics_required = !config.analysis_file.empty();
        const bool is_binary_required = !config.binary_output_file.empty();
        const bool is_livereg_required = !config.livereg_analysis_file.empty();
        const bool is_cfg_required = (!config.inst_cfg_file.empty() || !config.block_cfg_file.empty());
        std::vector <AnalysisData> analysis_data;
        std::vector <std::string> device_analysis_data;

        // Check flags first.
        if (config.mode == RgaMode::kModeDx11 && config.profile.length() == 0 && !config.dxbc_input_dx11)
        {
            std::stringstream log;
            log << "-p Must be specified. Check compiler target: vs_5_0, ps_5_0 etc.";
            LogCallback(log.str());
            return;
        }

        if (config.mode != RgaMode::kModeDx11 && config.mode != RgaMode::kModeAmdil)
        {
            std::stringstream log;
            log << "Source language is not supported. Please use ";
            LogCallback(log.str());
            return;
        }

        // Run FXC if required. It must be first because this is the input for the compilation. we cannot check for success.
        if (config.fxc.length() > 0)
        {
            std::string fixed_cmd("\"");
            fixed_cmd += config.fxc;
            fixed_cmd += "\"";
            int rc = ::system(fixed_cmd.c_str());
            if (rc != 0)
            {
                std::stringstream log;
                log << "FXC failed. Please check the arguments and path are correct. If path contains spaces, you need to put it in \\\"\\\" for example\n";
                log << "-f  VsMain1 -s DXAsm -p vs_5_0 c:\\temp\\ClippingBlob.obj  --isa c:\\temp\\dxTest.isa -c tahiti --FXC \"\\\"C:\\Program Files (x86)\\Windows Kits\\8.1\\bin\\x86\\fxc.exe\\\" /E VsMain1 /T vs_5_0  /Fo c:/temp/ClippingBlob.obj c:/temp/Clipping.fx\" ";
                LogCallback(log.str());
                return;
            }
        }

        // see if the user asked for specific asics
        InitRequestedAsicListDX(config);

        // We need to iterate over the selected devices
        bool is_compile_success = false;
        for (const GDT_GfxCardInfo& devce_info : dx_default_asics_list_)
        {
            // Get the device name.
            std::string device_name = devce_info.m_szCALName;
            if (Compile(config, devce_info, device_name))
            {
                if (is_binary_required)
                {
                    ExtractBinary(device_name, config);
                }

                std::string isa_buffer;
                is_compile_success = true;
                bool is_isa_size_detected = false;
                bool should_detect_isa_size = true;
                size_t isa_size_bytes(0);
                if (is_isa_required || is_statistics_required || is_livereg_required || is_cfg_required)
                {
                    ExtractISA(device_name, config, isa_size_bytes, isa_buffer, is_isa_size_detected, should_detect_isa_size);
                }
                if (is_il_required)
                {
                    ExtractIL(device_name, config);
                }
                if (is_statistics_required)
                {
                    is_isa_size_detected = ExtractStats(device_name, config, should_detect_isa_size, isa_buffer, is_isa_size_detected,
                        isa_size_bytes, analysis_data, device_analysis_data);
                }
            }

            std::stringstream log;
            LogCallback(log.str());
        }

        if ((analysis_data.size() > 0) && is_compile_success)
        {
            gtString analysis_filename;
            KcUtils::ConstructOutputFileName(config.analysis_file, kStrDefaultExtensionStats,
                kStrDefaultExtensionCsv, config.function, kStrAllDevicesToken, analysis_filename);

            std::stringstream s_Log;
            WriteAnalysisDataForDX(config, analysis_data, device_analysis_data, analysis_filename.asASCIICharArray(), s_Log);
            LogCallback(s_Log.str());
        }

        // We should do this only once because it is the same to all devices.
        if ((config.dump_ms_intermediate.size() > 0) && is_compile_success)
        {
            std::string ms_intermediate;
            beStatus backend_rc = backend_handler_->theOpenDXBuilder()->GetIntermediateMsBlob(ms_intermediate);
            if (backend_rc == kBeStatusSuccess)
            {
                std::stringstream ss;
                ss << kStrInfoDx11DxAsmCodeGenerationSuccess << std::endl;
                LogCallback(ss.str());

                gtString dx_asm_output_filename;
                KcUtils::ConstructOutputFileName(config.dump_ms_intermediate, "", kStrDefaultExtensionDxasm,
                    config.function, "", dx_asm_output_filename);
                KcUtils::WriteTextFile(dx_asm_output_filename.asASCIICharArray(), ms_intermediate, log_callback_);
            }
            else
            {
                std::stringstream ss;
                ss << kStrInfoDx11DxAsmCodeGenerationFailure << std::endl;
                LogCallback(ss.str());
            }
        }
    }
}

bool KcCliCommanderDX::WriteAnalysisDataForDX(const Config& config, const std::vector<AnalysisData>& analysis_data,
    const std::vector<string>& device_analysis_data, const std::string& analysis_file, std::stringstream& log)
{
    // Get the separator for CSV list items.
    char csv_separator = KcUtils::GetCsvSeparator(config);

    // Open output file.
    std::ofstream output;
    output.open(analysis_file);
    if (!output.is_open())
    {
        log << "Error: Unable to open " << analysis_file << " for write.\n";
    }
    else
    {
        // Write the headers.
        output << KcUtils::GetStatisticsCsvHeaderString(csv_separator) << std::endl;

        // Write the statistics data.
        if (!analysis_data.empty())
        {
            std::vector<std::string>::const_iterator iter = device_analysis_data.begin();
            std::vector<AnalysisData>::const_iterator iter_ad = analysis_data.begin();

            for (; iter < device_analysis_data.end(); ++iter, ++iter_ad)
            {
                // Device name.
                output << *iter << csv_separator;

                // Get the Analysis item.
                const AnalysisData& ad = *iter_ad;

                // Scratch registers.
                output << ad.scratch_memory_used << csv_separator;

                // Work-items per work-group.
                output << ad.num_threads_per_group_total << csv_separator;

                // Wavefront size.
                output << ad.wavefront_size << csv_separator;

                // LDS available bytes.
                output << ad.lds_size_available << csv_separator;

                // LDS actual bytes.
                output << ad.lds_size_used << csv_separator;

                // Available SGPRs.
                output << ad.num_sgprs_available << csv_separator;

                // Used SGPRs.
                output << ad.num_sgprs_used << csv_separator;

                // SGPR spills.
                output << kStrDx11NaValue << csv_separator;

                // Available VGPRs.
                output << ad.num_vgprs_available << csv_separator;

                // Used VGPRs.
                output << ad.num_vgprs_used << csv_separator;

                // VGPR spills.
                output << kStrDx11NaValue << csv_separator;

                // CL Work-group dimensions (for a unified format, to be revisited).
                output << ad.num_threads_per_group_x << csv_separator;
                output << ad.num_threads_per_group_y << csv_separator;
                output << ad.num_threads_per_group_z << csv_separator;

                // ISA size.
                output << ad.isa_size;

                output << endl;
            }
        }

        output.close();
    }

    return true;
}

bool ParseProfileString(const std::string& profile, std::pair<int, int>& version)
{
    bool result = false;

    // Profile string format: XX_N_N.
    if (!profile.empty())
    {
        size_t  minor, major = profile.find('_');
        if (major != std::string::npos)
        {
            if ((minor = profile.find('_', ++major)) != std::string::npos)
            {
                try
                {
                    version = { std::stoi(profile.substr(major, minor)), std::stoi(profile.substr(++minor)) };
                    result = true;
                }
                catch (...) {}
            }
        }
    }
    return result;
}

/// Output the ISA representative of the compilation
bool KcCliCommanderDX::Compile(const Config& config, const GDT_GfxCardInfo& gfxCardInfo, const std::string& sDevicenametoLog)
{
    bool ret = true;
    std::stringstream log;
    std::pair<int, int> version;
    Dx11Options dx_options;

    if (config.mode != RgaMode::kModeAmdil && !config.dxbc_input_dx11 &&
        !ParseProfileString(config.profile, version))
    {
        log << kStrErrorDx11IncorrectShaderModel << std::endl;
        ret = false;
    }

    if ((config.mode == RgaMode::kModeAmdil || config.dxbc_input_dx11) && !config.profile.empty())
    {
        std::cout << kStrWarningDx11ShaderModelIgnored << std::endl;
    }

    // Maximum  supported shader model version is 5.0.
    bool is_shader_model_supported = !(version.first > 5 || (version.first == 5 && version.second >= 1));
    if (is_shader_model_supported)
    {
        // Check the provided shader profile version.
        if (ret)
        {
            if (version.first > kDxMaxSupportedShaderModelMajor ||
                (version.first == kDxMaxSupportedShaderModelMajor && version.second > kDxMaxSupportedShaderModelMinor))
            {
                log << kStrErrorDx11UnsupportedShaderModel1 << config.profile << ". "
                    << kStrErrorDx11UnsupportedShaderModel2 << kDxMaxSupportedShaderModelMajor << "." << kDxMaxSupportedShaderModelMinor << " "
                    << kStrErrorDx11UnsupportedShaderModel3 << std::endl;
                ret = false;
            }

            if ((config.profile.find("_3_") != std::string::npos) || (config.profile.find("_2_") != std::string::npos) || (config.profile.find("_1_") != std::string::npos)
                || (config.profile.find("level_9_") != std::string::npos))
            {
                log << kStrWarningDx11MinSupportedVersion << std::endl;
            }
        }

        if (ret)
        {
            // Prepare the options.
            dx_options.entrypoint = config.function;
            dx_options.target = config.profile;
            dx_options.dx_flags.flags_as_int = config.dx_flags;
            dx_options.should_dump_ms_intermediate = (config.dump_ms_intermediate.size() > 0 ? true : false);
            dx_options.is_shader_intrinsics_enabled = config.enable_shader_intrinsics;
            dx_options.uav_slot = config.uav_slot;

            // Process config.m_Defines
            // The interface for DX is different here.
            // It is set up for the GUI.
            // The CLI does some work here to translate.
            for (std::vector<std::string>::const_iterator it = config.defines.begin();
                it != config.defines.end();
                ++it)
            {
                size_t equal_pos = it->find('=');
                if (equal_pos == std::string::npos)
                {
                    dx_options.defines.push_back(make_pair(*it, std::string("")));
                }
                else
                {
                    dx_options.defines.push_back(make_pair(it->substr(0, equal_pos),
                        it->substr(equal_pos + 1, std::string::npos)));
                }
            }
        }

        // Read the source code.
        std::string source_code;
        if (ret)
        {
            if (!config.input_files.empty())
            {
                if (config.input_files.size() > 1)
                {
                    log << kStrErrorSingleInputFileExpected << std::endl;
                    ret = false;
                }
                else
                {
                    ret = KcUtils::ReadProgramSource(config.input_files[0], source_code);
                    if (!ret)
                    {
                        log << kStrErrorCannotReadFile << config.input_files[0] << std::endl;
                    }
                }
            }
            else
            {
                log << kStrErrorNoInputFile << std::endl;
                ret = false;
            }
        }

        if (ret)
        {
            beStatus backend_rc = backend_handler_->GetDeviceChipFamilyRevision(gfxCardInfo, dx_options.chip_family, dx_options.chip_revision);
            if (backend_rc != kBeStatusSuccess)
            {
                log << "Error: could not find device named: " << sDevicenametoLog << ". Run \'-s dx11 -l to view available devices." << endl;
                ret = false;
            }
            else
            {
                // Set the source code file name.
                dx_options.filename = config.input_files[0];

                // Set the device file name.
                dx_options.device_name = sDevicenametoLog;

                // Fill the include files path.
                for (const std::string& include_path : config.include_path)
                {
                    dx_options.include_directories.push_back(include_path);
                }

                backend_rc = backend_handler_->theOpenDXBuilder()->Compile(config.mode,
                    source_code, dx_options, config.dxbc_input_dx11);

                if (backend_rc == kBeStatusCreateBlobFromFileFailed)
                {
                    log << "Error reading DX ASM file. ";
                    ret = false;
                }

                if (backend_rc != kBeStatusSuccess)
                {
                    log << kStrInfoCompiling << sDevicenametoLog << "... " << kStrInfoFailed << std::endl;
                    ret = false;
                }
                else
                {
                    log << kStrInfoCompiling << sDevicenametoLog << "... " << kStrInfoSuccess << std::endl;
                    ret = true;
                }
            }
        }
    }
    else
    {
        std::cout << kStrErrorShaderModelNotSupported << std::endl;
        ret = false;
    }

    LogCallback(log.str());
    return ret;
}

bool KcCliCommanderDX::Init(const Config& config, LoggingCallBackFuncP callback)
{
    bool should_continue = true;
    log_callback_ = callback;

    // Initialize backend.
    backend_handler_ = Backend::Instance();

    if (!backend_handler_->Initialize(kProgramTypeDx11, callback))
    {
        should_continue = false;
    }

    BeProgramBuilderDx11& dx_builder = *(backend_handler_->theOpenDXBuilder());

    if (should_continue)
    {
        // Initialize the DX Builder
        dx_builder.SetLog(callback);

        // If a particular GPU adapter is requested, choose corresponding driver.
        std::string  adapter_name = "", dxxModulePath;
        if (config.dx_adapter != -1)
        {
            should_continue = BeProgramBuilderDx11::GetDxxModulePathForAdapter(config.dx_adapter, config.print_process_cmd_line, adapter_name, dxxModulePath);
        }

        if (should_continue)
        {
            should_continue = (dx_builder.Initialize(dxxModulePath, config.dx_compiler_location) == beKA::kBeStatusSuccess);
        }
        else
        {
            std::stringstream  error_msg;
            error_msg << kStrErrorDx11AdapterSetFailed << std::endl;
            callback(error_msg.str());
        }
    }

    if (should_continue)// init ASICs list
    {
        std::vector<GDT_GfxCardInfo> dx_device_table;
        beStatus backend_rc = dx_builder.GetDeviceTable(dx_device_table);

        // Returns true if the device is disabled for all modes.
        auto is_disabled = [](const std::string& device)
        { for (auto& d : KcUtils::GetRgaDisabledDevices()) { if (d.find(device) != std::string::npos) return true; } return false; };

        if (backend_rc == kBeStatusSuccess)
        {
            std::string cal_name;
            for (vector<GDT_GfxCardInfo>::const_iterator it = dx_device_table.begin(); it != dx_device_table.end(); ++it)
            {
                if (cal_name != std::string(it->m_szCALName) && !is_disabled(it->m_szCALName))
                {
                    cal_name = it->m_szCALName;
                    dx_default_asics_list_.push_back(*it);
                }
            }
        }
        else
        {
            should_continue = false;
        }
    }

    return should_continue;
}

#endif
