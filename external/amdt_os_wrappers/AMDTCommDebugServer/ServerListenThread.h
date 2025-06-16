//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ServerListenThread.h
///
//=====================================================================

#pragma once

#include <list>
#include <amdt_os_wrappers/Include/osThread.h>
#include <amdt_os_wrappers/Include/osTCPSocketServer.h>

class ServerWorkerThread;

class ServerListenThread : public osThread
{
public:
    ServerListenThread();
    virtual ~ServerListenThread();
    bool init(int portNum);

protected:
    virtual int entryPoint();
    virtual void beforeTermination();

private:
    osTCPSocketServer m_serverSocket;
    std::list<ServerWorkerThread*> m_spawnedThreads;
    bool m_bContinueRunning;
};
