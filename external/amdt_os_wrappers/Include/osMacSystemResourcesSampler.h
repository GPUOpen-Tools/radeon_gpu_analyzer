//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMacSystemResourcesSampler.h
///
//=====================================================================

//------------------------------ osMacSystemResourcesSampler.h ------------------------------

#ifndef __OSMACSYSTEMRESOURCESSAMPLER_H
#define __OSMACSYSTEMRESOURCESSAMPLER_H

// Infra:
#include <amdt_base_tools/Include/gtVector.h>

// Local:
#include <amdt_os_wrappers/Include/osOSWrappersDLLBuild.h>
#include <amdt_os_wrappers/Include/osSystemResourcesDataSampler.h>
#include <amdt_os_wrappers/Include/osCPUSampledData.h>


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osMacSystemResourcesSampler : public osSystemResourcesDataSampler
// General Description: A class used to sample CPU and memory data on the Mac
// Author:      AMD Developer Tools Team
// Creation Date:        22/11/2009
// ----------------------------------------------------------------------------------
class OS_API osMacSystemResourcesSampler : public osSystemResourcesDataSampler
{
public:
    osMacSystemResourcesSampler();
    virtual ~osMacSystemResourcesSampler();

    // CPUs
    virtual bool updateCPUsData();
    virtual int cpusAmount() const;
    virtual bool getGlobalCPUData(osCPUSampledData& cpuStatistics) const;
    virtual bool getCPUData(int cpuIndex, osCPUSampledData& cpuStatistics) const;

    // Memory
    virtual bool updatePhysicalMemoryData();
    virtual bool getPhysicalMemoryData(osPhysicalMemorySampledData& memoryStatistics) const;

private:
    // Number of CPUs this class samples:
    int _amountOfCPUs;

    // The last sampled CPU data:
    gtVector<osCPUSampledData> _cpusSampledData;

};

#endif //__OSMACSYSTEMRESOURCESSAMPLER_H

