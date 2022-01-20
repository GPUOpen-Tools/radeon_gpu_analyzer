//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTransferableObjectType.cpp
///
//=====================================================================

//------------------------------ osTransferableObjectType.cpp ------------------------------

// Infra:
#include <amdt_base_tools/Include/gtAssert.h>

// Local:
#include <amdt_os_wrappers/Include/osChannel.h>
#include <amdt_os_wrappers/Include/osTransferableObjectType.h>

// Data types sizes:
static unsigned long statTransferableObjTypeSize = sizeof(osTransferableObjectType);


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes an osTransferableObjectType enum into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        3/5/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, osTransferableObjectType objType)
{
    const gtByte* pDataBuffer = (const gtByte*)&objType;
    bool rc = ipcChannel.write(pDataBuffer, statTransferableObjTypeSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads an osTransferableObjectType enum from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        3/5/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, osTransferableObjectType& objType)
{
    gtByte* pDataBuffer = (gtByte*)&objType;
    bool rc = ipcChannel.read(pDataBuffer, statTransferableObjTypeSize);
    GT_ASSERT(rc);

    return ipcChannel;
}

