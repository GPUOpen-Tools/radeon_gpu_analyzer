//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCallsStackReader.cpp
///
//=====================================================================

//------------------------------ osCallsStackReader.cpp ------------------------------

// Infra:
#include <amdt_base_tools/Include/gtAssert.h>

// Local:
#include <amdt_os_wrappers/Include/osCallsStackReader.h>
#include <amdt_os_wrappers/Include/osCallStack.h>
#include <amdt_os_wrappers/Include/osWin32CallStackReader.h>
#include <amdt_os_wrappers/Include/osWin32DebugSymbolsManager.h>

// ---------------------------------------------------------------------------
// Name:        osCallsStackReader::osCallsStackReader
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        23/10/2008
// ---------------------------------------------------------------------------
osCallsStackReader::osCallsStackReader()
{

}

// ---------------------------------------------------------------------------
// Name:        osCallsStackReader::~osCallsStackReader
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        23/10/2008
// ---------------------------------------------------------------------------
osCallsStackReader::~osCallsStackReader()
{

}

// ---------------------------------------------------------------------------
// Name:        osCallsStackReader::getCurrentCallsStack
// Description: populates callStack with the current thread's calls stack. If
//              shouldIgnoreSpyFrames is true, stack frames belonging to the
//              spy are discarded.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/10/2008
// ---------------------------------------------------------------------------
bool osCallsStackReader::getCurrentCallsStack(osCallStack& callStack, bool shouldIgnoreSpyFrames, bool forceWindowsSymbolLoading)
{
    bool retVal = false;

    if (forceWindowsSymbolLoading)
    {
        // If this is the first time we are getting a call stack for the current process,
        // initialize the symbol server:
        static bool loadSymbols = true;

        if (loadSymbols)
        {
            loadSymbols = false;
            osWin32DebugSymbolsManager symMgr;
            symMgr.initializeProcessSymbolsServer(GetCurrentProcess(), true);
        }
    }

    osWin32CallStackReader callsStackReaderImpl(callStack);
    retVal = callsStackReaderImpl.execute(shouldIgnoreSpyFrames);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCallsStackReader::getCallStack
// Description: Inputs a thread's execution context and outputs the call stack
//              associated with it.
// Arguments:
//  callStack - Will get the output call stack.
//  shouldIgnoreSpyFrames - If true, the output call stack will not include
//                          spy related call stack threds.
//  pThreadExecutionContext - A pointer to a thread's execution context
//                                      cast into void*.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/5/2009
// ---------------------------------------------------------------------------
bool osCallsStackReader::getCallStack(osCallStack& callStack, void* pThreadExecutionContext, bool shouldIgnoreSpyFrames)
{
    GT_UNREFERENCED_PARAMETER(shouldIgnoreSpyFrames);
    osProcessHandle hCurrentProcsss = GetCurrentProcess();
    osWin32CallStackReader callsStackReaderImpl(hCurrentProcsss, (CONTEXT*)pThreadExecutionContext, callStack);
    bool retVal = callsStackReaderImpl.execute(false);
    return retVal;
}

