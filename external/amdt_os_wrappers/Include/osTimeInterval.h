//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTimeInterval.h
///
//=====================================================================

#ifndef __OSTIMEINTERVAL_H
#define __OSTIMEINTERVAL_H

// Infra:
#include <amdt_base_tools/Include/AMDTDefinitions.h>
#include <amdt_base_tools/Include/gtString.h>

// Local:
#include <amdt_os_wrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           osTimeInterval
// General Description: Represents a time interval (amount of time that exists
//                      between two absolute data and time).
// Author:      AMD Developer Tools Team
// Creation Date:        17/5/2003
// ----------------------------------------------------------------------------------
class OS_API osTimeInterval
{
public:

    /// Constructor:
    osTimeInterval();
    osTimeInterval(const gtUInt64& intervalInNanoSeconds);

    void getAsMilliSeconds(double& intervalInMilliSeconds) const;
    void setAsMilliSeconds(const double& intervalInMilliSeconds);
    void getAsWholeSecondsAndRemainder(gtUInt64& seconds, gtUInt64& nanoSecondsRemainder) const;

    /// Convert time interval to string:
    gtString AsString() const;

private:

    gtUInt64 m_timeIntervalNanoSeconds;
};



#endif //__OSTIMEINTERVAL_H

