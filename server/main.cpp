#include <cstdio>
#include "RPCServer.h"

int main(int argc, char const* argv[])
{

    const char* serverIP = argv[1];
    int port = atoi(argv[2]);

    bool statusOk = true;
    RPCServer* serverObj = new RPCServer(serverIP, port);

#if 0
    std::vector<std::string> tokArray;
    char buffer[128] = { "connect;mike;mike" };
    serverObj->ParseTokens(buffer, tokArray);
#endif
    // Print it out

    statusOk = serverObj->StartServer();
    printf("Endless loop, as server is up always");
    while (statusOk)
    {
        statusOk = serverObj->ListenForClient();
        //statusOk = serverObj->ProcessRPC(); // Launch thread
    }

    delete serverObj;
    return 0;
}