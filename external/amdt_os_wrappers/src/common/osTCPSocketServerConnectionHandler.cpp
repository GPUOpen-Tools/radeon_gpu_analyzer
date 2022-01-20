//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTCPSocketServerConnectionHandler.cpp
///
//=====================================================================

//------------------------------ osTCPSocketServerConnectionHandler.cpp ------------------------------

// GRBaseTools:
#include <amdt_base_tools/Include/gtAssert.h>

// Local:
#include <amdt_os_wrappers/Include/osTCPSocketServerConnectionHandler.h>

// ---------------------------------------------------------------------------
// Name:        osTCPSocketServerConnectionHandler::osTCPSocketServerConnectionHandler
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
osTCPSocketServerConnectionHandler::osTCPSocketServerConnectionHandler()
    : osTCPSocket(L"osTCPSocketServerConnectionHandler")
{
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketServerConnectionHandler::~osTCPSocketServerConnectionHandler
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        4/1/2004
// ---------------------------------------------------------------------------
osTCPSocketServerConnectionHandler::~osTCPSocketServerConnectionHandler()
{
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketServerConnectionHandler::initialize
// Description: Initialize me using an OS socket descriptor
// Author:      AMD Developer Tools Team
// Date:        18/1/2004
// ---------------------------------------------------------------------------
void osTCPSocketServerConnectionHandler::initialize(osSocketDescriptor socketDescriptor)
{
    setOSDescriptor(socketDescriptor);
}
