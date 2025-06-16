//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFileLauncherThread.h
///
//=====================================================================

//------------------------------ osFileLauncherThread.h ------------------------------

#ifndef __OSFILELAUNCHERTHREAD
#define __OSFILELAUNCHERTHREAD

// Infra:
#include <amdt_base_tools/Include/gtString.h>

// Local:
#include <amdt_os_wrappers/Include/osThread.h>

// ----------------------------------------------------------------------------------
// Class Name:           osFileLauncherThread : public osThread
// General Description: A thread that launch a file using the machine default
//                      "open" application for that file.
// Author:      AMD Developer Tools Team
// Creation Date:        24/3/2005
// ----------------------------------------------------------------------------------
class osFileLauncherThread : public osThread
{
public:
    osFileLauncherThread(const gtString& fileToBeLaunched);
    osFileLauncherThread(const gtString& fileToBeLaunched, const gtString& launchParameters);

protected:
    // Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination();

private:
    // Do not allow the use of my default constructor:
    osFileLauncherThread();

private:
    // The path / URL of the file to be launched:
    gtString _fileToBeLaunched;
    gtString _launchParameters;
};


#endif  // __OSFILELAUNCHERTHREAD
