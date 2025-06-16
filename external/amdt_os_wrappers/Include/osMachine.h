//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMachine.h
///
//=====================================================================

//------------------------------ osMachine.h ------------------------------

#ifndef __OSMACHINE
#define __OSMACHINE

// Infra:
#include <amdt_base_tools/Include/gtString.h>
#include <amdt_base_tools/Include/gtASCIIString.h>

// Local:
#include <amdt_os_wrappers/Include/osOSWrappersDLLBuild.h>
#include <amdt_os_wrappers/Include/osOSDefinitions.h>


OS_API bool osGetLocalMachineName(gtString& localMachineName);
OS_API bool osGetLocalMachineName(gtASCIIString& localMachineName);
OS_API bool osGetAmountOfLocalMachineCPUs(int& amountOfCPUs);
OS_API bool osGetLocalMachineMemoryLimits(gtSize_t& physicalMemAmount, gtSize_t& virtualMemAmount);
OS_API bool osGetLocalMachineVirtualMemPageSize(unsigned long& pageSize);
OS_API bool osGetLocalMachineMemoryInformationStrings(gtString& totalRam, gtString& availRam, gtString& totalPage, gtString& availPage, gtString& totalVirtual, gtString& availVirtual);
OS_API bool osGetLocalMachineMemoryInformation(gtUInt64& totalRam, gtUInt64& availRam, gtUInt64& totalPage, gtUInt64& availPage, gtUInt64& totalVirtual, gtUInt64& availVirtual);
OS_API bool osGetLocalMachineCPUInformationStrings(gtString& numberOfProcessors, gtString& processorType);
OS_API bool osGetLocalMachineSystemPathAndDelims(gtString& computerPath, gtString& computerPathDelims);
OS_API bool osGetLocalMachineUserAndDomain(gtString& userName, gtString& userDomain);

#endif // __OSMACHINE
