/* RPCServer.cpp : This file contains the 'main' function.Program execution begins and ends there.
 This is a very very simple example of a CPSC5042 Server that will listen to
 a CPSC5042 Client
 Version 1 will have the server handle one client at a time. The server will:
 - Wait for connection from client
 - Process the Connect API  once connect
 - Process all RPC requests until the client does a Disconnect RPC
 - This intial server will handle 3 RPC's:
 -      Connect
 -      HowManyChars (will return HowManyChars are in the input message)
 -      Disconnnect
 server, then run the various RPC's that might happen between the server and
 client
*/


#pragma once
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <vector>
#include <iterator>


class RPCServer
{
public:
    RPCServer(const char *serverIP, int port);
    ~RPCServer();
    bool StartServer();
    bool ListenForClient();

    void ParseTokens(char* buffer, std::vector<std::string>& a);

private:
    int m_rpcCount;
    int m_server_fd;
    int m_socket;
    char* m_serverIP;
    int m_port;
    struct sockaddr_in m_address;
    int playerID = 0;
    const int MaxPlayerNumber = 10;



};

