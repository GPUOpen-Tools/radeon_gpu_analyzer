#include <cstdio>
#include <iostream>

#include <amdt_base_tools/Include/gtString.h>
#include <amdt_os_wrappers/Include/osConsole.h>
#include "ServerListenThread.h"

int main(int argc, char* argv[])
{
    const int DEFAULT_PORT_NUMBER = 10000;
    int portNum = 0;
    std::wcout << L"AMDTCommDebugServer running, press any key to exit." << std::endl;

    if (argc > 1)
    {
        gtString portNumString;
        portNumString.fromASCIIString(argv[1], strlen(argv[1]));
        portNumString.toIntNumber(portNum);
    }
    else
    {
        portNum = DEFAULT_PORT_NUMBER;
    }

    ServerListenThread listenThread;
    listenThread.init(portNum);
    listenThread.execute();

    // Wait for user interaction on main thread.
    osWaitForKeyPress();

    listenThread.terminate();
}
