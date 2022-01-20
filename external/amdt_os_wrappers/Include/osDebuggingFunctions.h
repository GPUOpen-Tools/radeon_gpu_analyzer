//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDebuggingFunctions.h
///
//=====================================================================

//------------------------------ osDebuggingFunctions.h ------------------------------

#ifndef __OSDEBUGGINGFUNCTIONS
#define __OSDEBUGGINGFUNCTIONS

// Forward decelerations:
class osFilePath;

// Infra:
#include <amdt_base_tools/Include/gtString.h>

// Local:
#include <amdt_os_wrappers/Include/osOSWrappersDLLBuild.h>

#define OS_STR_DebugStringOutputPrefix L"Debug string: "

OS_API void osOutputDebugString(const gtString& debugString);
OS_API void osWPerror(const wchar_t* pErrorMessage);
OS_API void osThrowBreakpointException();
OS_API bool osIsRunningUnderDebugger();
OS_API bool osOpenFileInSourceCodeEditor(const osFilePath& filePath, int lineNumber);

#endif  // __OSDEBUGGINGFUNCTIONS
