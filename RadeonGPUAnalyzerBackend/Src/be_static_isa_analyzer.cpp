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
#include "AMDTOSWrappers/Include/osProcess.h"
#include "AMDTOSWrappers/Include/osFilePath.h"
#include "AMDTOSWrappers/Include/osFile.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include "RadeonGPUAnalyzerBackend/Src/be_static_isa_analyzer.h"
#include "RadeonGPUAnalyzerBackend/Src/be_utils.h"

// CLI.
#include "RadeonGPUAnalyzerCLI/Src/kc_utils.h"

using namespace beKA;

// Static constants.
static const std::string kShaeOptLivereg = "analyse-liveness";
static const std::string kShaeOptCfgPerBlock = "dump-bb-cfg";
static const std::string kShaeOptCfgPerInstruciton = "dump-pi-cfg";
static const std::string kShaeOptStallAnalysis = "stall-cycles";

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

static beKA::beStatus PerformAnalysis(const gtString& isa_filename, const gtString& output_filename,
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
            cmd << analyzer_path << " " << shae_cmd << " \"" << isa_filename.asASCIICharArray()
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

beKA::beStatus beKA::BeStaticIsaAnalyzer::PerformLiveRegisterAnalysis(const gtString& isa_filename,
    const gtString& output_filename, bool should_print_cmd)
{
    return PerformAnalysis(isa_filename, output_filename, kShaeOptLivereg, should_print_cmd);
}

beKA::beStatus BeStaticIsaAnalyzer::PerformStallAnalysis(const gtString& isa_filename, const gtString& output_filename,
    bool should_print_cmd)
{
    return PerformAnalysis(isa_filename, output_filename, kShaeOptStallAnalysis, should_print_cmd);
}

beKA::beStatus beKA::BeStaticIsaAnalyzer::GenerateControlFlowGraph(const gtString& isa_filename, const gtString& output_filename,
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
            cmd << analyzer_path << " " << kShaeOptCfg << " " << '"' << isa_filename.asASCIICharArray()
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
