//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
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
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beStaticIsaAnalyzer.h>
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>

// CLI.
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>

using namespace beKA;

// Static constants.
static const std::string  RGA_SHAE_OPT_LIVEREG   = "analyse-liveness";
static const std::string  RGA_SHAE_OPT_BLOCK_CFG = "dump-bb-cfg";
static const std::string  RGA_SHAE_OPT_INST_CFG  = "dump-pi-cfg";

static bool GetLiveRegAnalyzerPath(std::string& analyzerPath)
{
#ifdef AMD_INTERNAL

#ifdef __linux
    analyzerPath = "shae-internal";
#elif _WIN64
    analyzerPath = "x64\\shae-internal.exe";
#else
    analyzerPath = "x86\\shae-internal.exe";
#endif

#else

#ifdef __linux
    analyzerPath = "shae";
#elif _WIN64
    analyzerPath = "x64\\shae.exe";
#else
    analyzerPath = "x86\\shae.exe";
#endif

#endif // AMD_INTERNAL
    return true;
}

beKA::beStatus beKA::beStaticIsaAnalyzer::PreprocessIsaFile(const std::string& isaFileName, const std::string& outputFileName)
{
    beStatus ret = beStatus_General_FAILED;

    // Filter out the relevant lines.
    const char* SGPR_TOKEN = "sgpr_count";
    const char* VGPR_TOKEN = "vgpr_count";
    std::stringstream prcessedContent;
    std::ifstream infile(isaFileName);
    std::string line;
    while (std::getline(infile, line))
    {
        if (line.find(SGPR_TOKEN) == std::string::npos &&
            line.find(VGPR_TOKEN) == std::string::npos)
        {
            prcessedContent << line << std::endl;
        }
    }

    bool isContentValid = !prcessedContent.str().empty();
    assert(isContentValid);
    if (isContentValid)
    {
        bool isFileWritten = kcUtils::WriteTextFile(outputFileName , prcessedContent.str(), nullptr);
        assert(isFileWritten);
        if (isFileWritten)
        {
            ret = beStatus_SUCCESS;
        }
    }
    return ret;
}

beKA::beStatus beKA::beStaticIsaAnalyzer::PerformLiveRegisterAnalysis(const gtString& isaFileName, const gtString& outputFileName, bool printCmd)
{
    beStatus ret = beStatus_General_FAILED;

    // Get the ISA analyzer's path.
    std::string analyzerPath;
    bool isOk = GetLiveRegAnalyzerPath(analyzerPath);

    if (isOk && !analyzerPath.empty())
    {
        // Validate the input ISA file.
        osFilePath isaFilePath(isaFileName);

        if (isaFilePath.exists())
        {
            // Construct the command.
            std::stringstream cmd;
            cmd << analyzerPath << " " << RGA_SHAE_OPT_LIVEREG << " \"" << isaFileName.asASCIICharArray()
                << "\" \"" << outputFileName.asASCIICharArray() << '"';

            // Cancel signal. Not in use for now.
            bool shouldCancel = false;

            gtString analyzerOutput;
            beUtils::PrintCmdLine(cmd.str(), printCmd);
            isOk = osExecAndGrabOutput(cmd.str().c_str(), shouldCancel, analyzerOutput);

            if (isOk)
            {
                ret = beStatus_SUCCESS;
            }
            else
            {
                ret = beStatus_shaeFailedToLaunch;
            }
        }
        else
        {
            ret = beStatus_shaeIsaFileNotFound;
        }
    }
    else
    {
        ret = beStatus_shaeCannotLocateAnalyzer;
    }

    return ret;
}

beKA::beStatus beKA::beStaticIsaAnalyzer::GenerateControlFlowGraph(const gtString& isaFileName, const gtString& outputFileName,
                                                                   bool perInst, bool printCmd)
{
    beStatus ret = beStatus_General_FAILED;

    // Get the ISA analyzer's path.
    std::string analyzerPath;
    bool isOk = GetLiveRegAnalyzerPath(analyzerPath);

    if (isOk && !analyzerPath.empty())
    {
        // Validate the input ISA file.
        osFilePath isaFilePath(isaFileName);

        if (isaFilePath.exists())
        {
            // Construct the command.
            std::stringstream cmd;
            std::string shaeOptCfg = perInst ? RGA_SHAE_OPT_INST_CFG : RGA_SHAE_OPT_BLOCK_CFG;
            cmd << analyzerPath << " " << shaeOptCfg << " " << '"' << isaFileName.asASCIICharArray()
                << "\" \"" << outputFileName.asASCIICharArray() << '"';

            // Cancel signal. Not in use for now.
            bool shouldCancel = false;

            gtString analyzerOutput;
            beUtils::PrintCmdLine(cmd.str(), printCmd);
            isOk = osExecAndGrabOutput(cmd.str().c_str(), shouldCancel, analyzerOutput);

            if (isOk)
            {
                ret = beStatus_SUCCESS;
            }
            else
            {
                ret = beStatus_shaeFailedToLaunch;
            }
        }
        else
        {
            ret = beStatus_shaeIsaFileNotFound;
        }
    }
    else
    {
        ret = beStatus_shaeCannotLocateAnalyzer;
    }

    return ret;
}

beStaticIsaAnalyzer::beStaticIsaAnalyzer()
{
}


beStaticIsaAnalyzer::~beStaticIsaAnalyzer()
{
}
