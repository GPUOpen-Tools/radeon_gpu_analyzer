//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTime.cpp
///
//=====================================================================

//------------------------------ osTime.cpp ------------------------------
#include <time.h>
#include <amdt_base_tools/Include/gtAssert.h>
#include <amdt_base_tools/Include/gtString.h>


#include <amdt_os_wrappers/Include/osTime.h>

// ---------------------------------------------------------------------------
// Name:        currentPreciseTimeAsString
// Description: Get the current local time in string format, including milliseconds
// Arguments:   strTime         - output string
//              stringFormat    - the format that will applied to the string
// Return Val:  void
// Author:      Doron Ofek
// Date:        Dec-24, 2015
// ---------------------------------------------------------------------------
bool osTime::currentPreciseTimeAsString(gtString& strTime, TimeFormat stringFormat)
{
    struct timespec timePoint;
    bool retVal = false;

    int rc = clock_gettime(CLOCK_REALTIME, &timePoint);
    GT_IF_WITH_ASSERT(0 == rc)
    {
        osTime clockRealtime;
        clockRealtime.setTime(timePoint.tv_sec);

        clockRealtime.timeAsString(strTime, stringFormat, osTime::LOCAL);
        int milliseconds = timePoint.tv_nsec / 1000000;

        if (UNDERSCORE_SAPERATOR == stringFormat)
        {
            strTime.appendFormattedString(L"_%03d", milliseconds);
        }
        else
        {
            strTime.appendFormattedString(L".%03d", milliseconds);
        }
        retVal = true;
    }
    else
    {
        strTime.makeEmpty();
    }

    return retVal;
}
