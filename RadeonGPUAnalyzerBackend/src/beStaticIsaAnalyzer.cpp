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
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>

// Local.
#include <RadeonGPUAnalyzerBackend/include/beStaticIsaAnalyzer.h>

using namespace beKA;

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


beKA::beStatus beKA::beStaticIsaAnalyzer::PerformLiveRegisterAnalysis(const gtString& isaFileName, const gtString& outputFileName)
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
            cmd << analyzerPath << " analyse-liveness " << '"' << isaFileName.asASCIICharArray()
                << "\" \"" << outputFileName.asASCIICharArray() << '"';

            // Cancel signal. Not in use for now.
            bool shouldCancel = false;

            gtString analyzerOutput;
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

beKA::beStatus beKA::beStaticIsaAnalyzer::GenerateControlFlowGraph(const gtString& isaFileName, const gtString& outputFileName)
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
            cmd << analyzerPath << " dump-pi-cfg " << kcUtils::Quote(isaFileName.asASCIICharArray())
                << " " << kcUtils::Quote(outputFileName.asASCIICharArray());

            // Cancel signal. Not in use for now.
            bool shouldCancel = false;

            gtString analyzerOutput;
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
