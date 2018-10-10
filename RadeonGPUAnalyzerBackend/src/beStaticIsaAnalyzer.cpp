//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <string>
#include <sstream>

// Infra.
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>

// Local.
#include <RadeonGPUAnalyzerBackend/include/beStaticIsaAnalyzer.h>
#include <RadeonGPUAnalyzerBackend/include/beUtils.h>

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
