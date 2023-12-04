//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <cassert>
#include <string>
#include <sstream>
#include <fstream>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "external/amdt_os_wrappers/Include/osProcess.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osFile.h"
#include "external/amdt_os_wrappers/Include/osApplication.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include "radeon_gpu_analyzer_backend/be_static_isa_analyzer.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// CLI.
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"

using namespace beKA;

// Static constants.
static const std::string kShaeOptLivereg       = "analyse-liveness --arch-info";
static const std::string kShaeOptLiveregWave64 = "--wave-size 64";
static const std::string kShaeOptLiveregWave32 = "--wave-size 32";
static const std::string kShaeOptLiveregSgpr   = "--reg-type sgpr";

static const std::string kShaeOptCfgPerBlock = "dump-bb-cfg";
static const std::string kShaeOptCfgPerInstruciton = "dump-pi-cfg";

static bool GetLiveRegAnalyzerPath(std::string& analyzer_path)
{
#ifdef AMD_INTERNAL

#ifdef __linux
    analyzer_path = "shae-internal";
#elif _WIN64
    analyzer_path = "utils\\shae-internal.exe";
#else
    analyzer_path = "x86\\shae-internal.exe";
#endif

#else

#ifdef __linux
    analyzer_path = "shae";
#elif _WIN64
    analyzer_path = "utils\\shae.exe";
#else
    analyzer_path = "x86\\shae.exe";
#endif

#endif // AMD_INTERNAL
    return true;
}

// Accepts the target name and returns the relevant Shae --isa command.
static std::string GetShaeIsaCmd(const gtString& target)
{
    const gtString kShaeGfx8  = L"gfx8";
    const gtString kShaeGfx9  = L"gfx9";
    const gtString kShaeGfx90a  = L"gfx90a";
    const gtString kShaeGfx10_1 = L"gfx10_1";
	const gtString kShaeGfx10_3 = L"gfx10_3";
	const gtString kShaeGfx11 = L"gfx11";

	std::stringstream    shae_gfx_generation;
	shae_gfx_generation << "--isa ";
	if (KcUtils::IsNavi3Target(target.asASCIICharArray()))
	{
		shae_gfx_generation << kShaeGfx11.asASCIICharArray();
	}
	else if (KcUtils::IsNavi21AndBeyond(target.asASCIICharArray()))
	{
		shae_gfx_generation << kShaeGfx10_3.asASCIICharArray();
	}
    else if (KcUtils::IsNaviTarget(target.asASCIICharArray()))
    {
        shae_gfx_generation << kShaeGfx10_1.asASCIICharArray();
    }
    else if (KcUtils::IsVegaTarget(target.asASCIICharArray()))
    {
        if (KcUtils::IsMITarget(target.asASCIICharArray()))
        {
            shae_gfx_generation << kShaeGfx90a.asASCIICharArray();
        }
        else
        {
            shae_gfx_generation << kShaeGfx9.asASCIICharArray();
        }
    }
    else
    {
        // Fall back to GFX8.
        shae_gfx_generation << kShaeGfx8.asASCIICharArray();
    }
    return shae_gfx_generation.str().c_str();
}

static beKA::beStatus PerformAnalysis(const gtString& isa_filename, const gtString& target, const gtString& output_filename,
    const std::string& shae_cmd, bool should_print_cmd)
{
    beStatus ret = kBeStatusGeneralFailed;

    // Get the ISA analyzer's path.
    std::string analyzer_path;
    bool is_ok = GetLiveRegAnalyzerPath(analyzer_path);

    if (is_ok && !analyzer_path.empty())
    {
        // Validate the input ISA file.
        osFilePath isa_file_path(isa_filename);

        if (isa_file_path.exists())
        {
            // Construct the command.
            std::stringstream cmd;
            cmd << analyzer_path << " " << GetShaeIsaCmd(target) << " " << shae_cmd << " \"" << isa_filename.asASCIICharArray()
                << "\" \"" << output_filename.asASCIICharArray() << '"';

            // Cancel signal. Not in use for now.
            bool should_cancel = false;

            gtString analyzer_output;
            BeUtils::PrintCmdLine(cmd.str(), should_print_cmd);
            is_ok = osExecAndGrabOutput(cmd.str().c_str(), should_cancel, analyzer_output);

            if (is_ok)
            {
                ret = kBeStatusSuccess;
            }
            else
            {
                ret = kBeStatusShaeFailedToLaunch;
            }
        }
        else
        {
            ret = kBeStatusShaeIsaFileNotFound;
        }
    }
    else
    {
        ret = kBeStatusShaeCannotLocateAnalyzer;
    }

    return ret;
}

beKA::beStatus beKA::BeStaticIsaAnalyzer::PreprocessIsaFile(const std::string& isa_filename, const std::string& output_filename)
{
    beStatus ret = kBeStatusGeneralFailed;

    // Filter out the relevant lines.
    const char* kTokenSgpr = "sgpr_count";
    const char* kTokenVgpr = "vgpr_count";
    std::stringstream prcessed_content;
    std::ifstream infile(isa_filename);
    std::string line;
    while (std::getline(infile, line))
    {
        if (line.find(kTokenSgpr) == std::string::npos &&
            line.find(kTokenVgpr) == std::string::npos)
        {
            prcessed_content << line << std::endl;
        }
    }

    bool is_content_valid = !prcessed_content.str().empty();
    assert(is_content_valid);
    if (is_content_valid)
    {
        bool is_file_written = KcUtils::WriteTextFile(output_filename , prcessed_content.str(), nullptr);
        assert(is_file_written);
        if (is_file_written)
        {
            ret = kBeStatusSuccess;
        }
    }
    return ret;
}

beKA::beStatus beKA::BeStaticIsaAnalyzer::PerformLiveRegisterAnalysis(const gtString& isa_filename, const gtString& target,
                                                                      const gtString& output_filename,
                                                                      beWaveSize      wave_size,
                                                                      bool            should_print_cmd,
                                                                      bool            is_reg_type_sgpr)
{
    std::stringstream shae_option;
    switch (wave_size)
    {
    case kUnknown:
        // Let Shae deduce the wave size from the disassembly.
        shae_option << kShaeOptLivereg;
        if (is_reg_type_sgpr)
        {
            shae_option << " " << kShaeOptLiveregSgpr;
        }
        break;
    case kWave32:
        // Force wave32.
        shae_option << kShaeOptLivereg;
        if (is_reg_type_sgpr)
        {
            shae_option << " " << kShaeOptLiveregSgpr;
        }
        shae_option  << " " << kShaeOptLiveregWave32;
        break;
    case kWave64:
        // Force wave64.
        shae_option << kShaeOptLivereg;
        if (is_reg_type_sgpr)
        {
            shae_option << " " << kShaeOptLiveregSgpr;
        }
        shae_option << " " << kShaeOptLiveregWave64;
        break;
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }

    return PerformAnalysis(isa_filename, target, output_filename, shae_option.str(), should_print_cmd);
}

beKA::beStatus beKA::BeStaticIsaAnalyzer::GenerateControlFlowGraph(const gtString& isa_filename, const gtString& target, const gtString& output_filename,
    bool is_per_instruction, bool should_print_cmd)
{
    beStatus ret = kBeStatusGeneralFailed;

    // Get the ISA analyzer's path.
    std::string analyzer_path;
    bool is_ok = GetLiveRegAnalyzerPath(analyzer_path);

    if (is_ok && !analyzer_path.empty())
    {
        // Validate the input ISA file.
        osFilePath isa_file_path(isa_filename);

        if (isa_file_path.exists())
        {
            // Construct the command.
            std::stringstream cmd;
            const std::string kShaeOptCfg = is_per_instruction ? kShaeOptCfgPerInstruciton : kShaeOptCfgPerBlock;
            cmd << analyzer_path << " " << GetShaeIsaCmd(target) << " " << kShaeOptCfg << " " << '"' << isa_filename.asASCIICharArray()
                << "\" \"" << output_filename.asASCIICharArray() << '"';

            // Cancel signal. Not in use for now.
            bool should_cancel = false;

            gtString analyzer_output;
            BeUtils::PrintCmdLine(cmd.str(), should_print_cmd);
            is_ok = osExecAndGrabOutput(cmd.str().c_str(), should_cancel, analyzer_output);

            if (is_ok)
            {
                ret = kBeStatusSuccess;
            }
            else
            {
                ret = kBeStatusShaeFailedToLaunch;
            }
        }
        else
        {
            ret = kBeStatusShaeIsaFileNotFound;
        }
    }
    else
    {
        ret = kBeStatusShaeCannotLocateAnalyzer;
    }

    return ret;
}
