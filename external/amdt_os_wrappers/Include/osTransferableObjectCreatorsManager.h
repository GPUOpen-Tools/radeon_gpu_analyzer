//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTransferableObjectCreatorsManager.h
///
//=====================================================================

//------------------------------ osTransferableObjectCreatorsManager.h ------------------------------

#ifndef __OSTRANSFERABLEOBJECTCREATORSMANAGER
#define __OSTRANSFERABLEOBJECTCREATORSMANAGER

// GRBaseTools:
#include <amdt_base_tools/Include/gtVector.h>
#include <amdt_base_tools/Include/gtAutoPtr.h>

// Local:
#include <amdt_os_wrappers/Include/osTransferableObjectCreatorsBase.h>
#include <amdt_os_wrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osTransferableObjectCreatorsManager
// General Description:
//   Holds the transferable objects creators.
//   Enables the creation of a transferable object from a given transferable object
//   Id.
//
// Author:      AMD Developer Tools Team
// Creation Date:        31/1/2004
// ----------------------------------------------------------------------------------
class OS_API osTransferableObjectCreatorsManager
{
public:
    static osTransferableObjectCreatorsManager& instance();
    virtual ~osTransferableObjectCreatorsManager();

    void registerCreator(osTransferableObjectCreatorsBase& transferableObjectCreator);
    void registerAliasCreator(osTransferableObjectType aliasObjectType,  osTransferableObjectCreatorsBase& transferableObjectCreator);

    bool createObject(unsigned int objectType, gtAutoPtr<osTransferableObject>& aptrCreatedObject);

private:
    // Only my instance() method should create me:
    osTransferableObjectCreatorsManager();

    // Only osSingeltonsDelete should delete my single instance:
    friend class osSingeltonsDelete;

private:
    gtVector<osTransferableObjectCreatorsBase*> _idToTransferableObjCreator;

    // Pointer to this class single instance:
    static osTransferableObjectCreatorsManager* _pMySingleInstance;
};


#endif  // __OSTRANSFERABLEOBJECTCREATORSMANAGER
