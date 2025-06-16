//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osToAndFromString.h
///
//=====================================================================

//------------------------------ osToAndFromString.h ------------------------------

#ifndef __OSTOANDFROMSTRING_H
#define __OSTOANDFROMSTRING_H

// Infra:
#include <amdt_base_tools/Include/gtString.h>
#include <amdt_base_tools/Include/gtASCIIString.h>

// Local:
#include <amdt_os_wrappers/Include/osOSWrappersDLLBuild.h>
#include <amdt_os_wrappers/Include/osOSDefinitions.h>

OS_API bool osProcessIdToString(osProcessId processId, gtString& outString);
OS_API bool osProcessIdToString(osProcessId processId, gtASCIIString& outString);
OS_API bool osProcessIdFromString(const gtString& string, osProcessId& processId);


#endif //__OSTOANDFROMSTRING_H

