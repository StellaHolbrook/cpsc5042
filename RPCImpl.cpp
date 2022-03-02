#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <vector>
#include <iterator>
#include <iostream>
#include <fstream>
#include <array>

#include "RPCImpl.h"
#include "LocalContext.h"

//#define PORT 8081

using namespace std;

typedef struct _GlobalContext {
    int g_rpcCount;
} GlobalContext;

GlobalContext globalObj; // We need to protect this, as we don't want bad data

// other contextual structure

typedef struct _question {
    std::string body;
    std::string answerA;
    std::string answerB;
    std::string answerC;
    std::string answerD;
    char correctAnswer [2];
} question;

question currentQuestion; // protected instance of question in thread

// create an array of these structres
struct _question questionArray[3];
//int size = size(questionArray);

RPCImpl::RPCImpl(int socket)
{
    m_socket = socket;
    m_rpcCount = 0;
};

RPCImpl::~RPCImpl() {};
/*
* Going to populate a String vector with tokens extracted from the string the client sent.
* The delimter will be a ;
* An example buffer could be "connect;mike;mike;"
*/
void RPCImpl::ParseTokens(char* buffer, std::vector<std::string>& a)
{
    char* token;
    char* rest = (char*)buffer;

    while ((token = strtok_r(rest, ";", &rest)))
    {
        printf("%s\n", token);
        a.push_back(token);
    }

    return;
}


// once this gets called, heap collection will have values from text file
void RPCImpl::readFile(struct _question questionArray[])
{
    ifstream fin;
    fin.open ("Questions.txt");
    if (!fin.fail())
    {
        // this is hardcoded for now, is for file of 3 questions
        for (int i = 0; i < 2; i++)
        {
            string body, answerA, answerB, answerC, answerD, correctAnswer;
            //char correctAnswer[2];

            // use getline to get strucutre attributes
            getline(fin, body);
            getline(fin, answerA);
            getline(fin, answerB);
            getline(fin, answerC);
            getline(fin, answerD);
            getline(fin, correctAnswer);

            //assign to members
            questionArray[i].body = body;
            questionArray[i].answerA = answerA;
            questionArray[i].answerB = answerB;
            questionArray[i].answerC = answerC;
            questionArray[i].answerD = answerD;
            questionArray[i].correctAnswer[0] = correctAnswer[0];
            questionArray[i].correctAnswer[1] = correctAnswer[1];
        }
        fin.close();
    }
}


/*
* ProcessRPC will examine buffer and will essentially control
*/
bool RPCImpl::ProcessRPC()
{
    const char* rpcs[] = { "connect", "disconnect", "status" };
    char buffer[1024] = { 0 };
    std::vector<std::string> arrayTokens;
    int valread = 0;
    bool bConnected = false;
    bool bStatusOk = true;
    const int RPCTOKEN = 0;
    bool bContinue = true;

    while ((bContinue) && (bStatusOk))
    {
        // Should be blocked when a new RPC has not called us yet
        if ((valread = read(this->m_socket, buffer, sizeof(buffer))) <= 0)
        {
            printf("errno is %d\n", errno);
            break;
        }
        printf("%s\n", buffer);

        arrayTokens.clear();
        this->ParseTokens(buffer, arrayTokens);

        // Enumerate through the tokens. The first token is always the specific RPC
        for (vector<string>::iterator t = arrayTokens.begin(); t != arrayTokens.end(); ++t)
        {
            printf("Debugging our tokens\n");
            printf("token = %s\n", t);
        }

        // string statements are not supported with a switch, so using if/else logic to dispatch
        string aString = arrayTokens[RPCTOKEN];

        if ((bConnected == false) && (aString == "connect"))
        {
            bStatusOk = ProcessConnectRPC(arrayTokens);  // Connect RPC
            if (bStatusOk == true)
                bConnected = true;
        }

        else if ((bConnected == true) && (aString == "disconnect"))
        {
            bStatusOk = ProcessDisconnectRPC();
            printf("We are going to terminate this endless loop\n");
            bContinue = false; // We are going to leave this loop, as we are done
        }

        else if ((bConnected == true) && (aString == "status"))
            bStatusOk = ProcessStatusRPC();   // Status RPC

        else
        {
            // Not in our list, perhaps, print out what was sent
        }

    }

    return true;
}

bool RPCImpl::ProcessConnectRPC(std::vector<std::string>& arrayTokens)
{
    const int USERNAMETOKEN = 1;
    const int PASSWORDTOKEN = 2;

    // Strip out tokens 1 and 2 (username, password)
    string userNameString = arrayTokens[USERNAMETOKEN];
    string passwordString = arrayTokens[PASSWORDTOKEN];
    char szBuffer[80];

    // Our Authentication Logic. Looks like Mike/Mike is only valid combination
    if ((userNameString == "MIKE") && (passwordString == "MIKE"))
    {
        strcpy(szBuffer, "1;"); // Connected
    }
    else
    {
        strcpy(szBuffer, "0;"); // Not Connected
    }

    // Send Response back on our socket
    int nlen = strlen(szBuffer);
    szBuffer[nlen] = 0;
    send(this->m_socket, szBuffer, strlen(szBuffer) + 1, 0);

    return true;
}

/* TDB
*/
bool RPCImpl::ProcessStatusRPC()
{
    return true;
}

/*
*/
bool RPCImpl::ProcessDisconnectRPC()
{
    char szBuffer[16];
    strcpy(szBuffer, "disconnect");
    // Send Response back on our socket
    int nlen = strlen(szBuffer);
    szBuffer[nlen] = 0;
    send(this->m_socket, szBuffer, strlen(szBuffer) + 1, 0);
    return true;
}