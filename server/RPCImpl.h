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
#include "LocalContext.h"
#include <map>

class RPCImpl
{
public:
    RPCImpl(int socket);
    ~RPCImpl();
    bool ProcessRPC();


private:
    int m_rpcCount;
    int m_socket;
    const int numOfQuestions = 10;
    LocalContext currentSocre;

    // First one in this function should be a connect, and it 
    // will continue try to process RPC's until a Disconnect happens
    bool ProcessConnectRPC(std::vector<std::string>& arrayTokens);
    bool ProcessGetQuestionRPC(std::vector<std::string>& arrayTokens);
    bool ProcessPostAnswerRPC(std::vector<std::string>& arrayTokens);
    bool ProcessGetScoreRPC(std::vector<std::string>& arrayTokens);
    bool ProcessDisconnectRPC();
    std::map<std::string, std::string> users();
    void ParseTokens(char* buffer, std::vector<std::string>& a);
    void readFile(struct _Question questionArray[]);
};

