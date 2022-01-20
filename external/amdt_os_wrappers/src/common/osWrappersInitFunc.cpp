//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osWrappersInitFunc.cpp
///
//=====================================================================

//------------------------------ osWrappersInitFunc.cpp ------------------------------

// Local:
#include <amdt_os_wrappers/Include/osTransferableObjectCreator.h>
#include <amdt_os_wrappers/Include/osTransferableObjectCreatorsManager.h>
#include <amdt_os_wrappers/Include/osFilePath.h>
#include <amdt_os_wrappers/Include/osDirectory.h>
#include <amdt_os_wrappers/Include/osWrappersInitFunc.h>


// ---------------------------------------------------------------------------
// Name:        apiClassesInitFunc
// Description: Initialization function for the GROSWrappers library.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        2/6/2004
// Implementation Notes:
//   Registeres all the GROSWrappers transferable objects in the transferable
//   objects creator manager.
// ---------------------------------------------------------------------------
bool osWrappersInitFunc()
{
    // Verify that this function code is executed only once:
    static bool wasThisFunctionCalled = false;

    if (!wasThisFunctionCalled)
    {
        wasThisFunctionCalled = true;

        // Get the osTransferableObjectCreatorsManager single instance:
        osTransferableObjectCreatorsManager& theTransfetableObsCreatorsManager = osTransferableObjectCreatorsManager::instance();

        // ----------- Register transferable objects creators -----------

        osTransferableObjectCreator<osFilePath> osFilePathCreator;
        theTransfetableObsCreatorsManager.registerCreator(osFilePathCreator);

        osTransferableObjectCreator<osDirectory> osDirectoryCreator;
        theTransfetableObsCreatorsManager.registerCreator(osDirectoryCreator);

    }

    return true;
}


