//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ServerWorkerThread.h
///
//=====================================================================

#pragma once

#include <amdt_os_wrappers/Include/osThread.h>
#include <amdt_os_wrappers/Include/osTCPSocketServer.h>
#include <amdt_os_wrappers/Include/osCriticalSection.h>

class osTCPSocketServerConnectionHandler;

class ServerWorkerThread : public osThread
{
public:
    ServerWorkerThread(osTCPSocketServerConnectionHandler* pClientCallHandler);
    ~ServerWorkerThread();

protected:
    virtual int entryPoint();
    virtual void beforeTermination();

private:
    osTCPSocketServerConnectionHandler* m_pClientCallHandler;
    bool m_bContinueRunning;
    static osCriticalSection m_lockWriteToConsole;
};
