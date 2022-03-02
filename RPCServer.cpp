

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <vector>
#include <iterator>

#include "RPCServer.h"
#include "RPCImpl.h"

//#define PORT 8081

using namespace std;



// A normal C function that is executed as a thread 
// when its name is specified in pthread_create()
void* myThreadFun(void* vargp)
{

    sleep(1);

    int socket = *(int *) vargp;
    printf("Printing GeeksQuiz from Thread \n");
    RPCImpl *rpcImplObj = new RPCImpl(socket);
    rpcImplObj->ProcessRPC();   // This will go until client disconnects;
    printf("Done with Thread");

    return NULL;

}

RPCServer::RPCServer(const char *serverIP, int port)
{
    m_rpcCount = 0; 
    m_serverIP = (char *) serverIP;
    m_port = port;
};

RPCServer::~RPCServer() {};

/*
* StartServer will create a server on a Port that was passed in, and create a socket
*/

bool RPCServer::StartServer()
{
    int opt = 1;
    const int BACKLOG = 10;


    // Creating socket file descriptor
    if ((m_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR,
        &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    m_address.sin_family = AF_INET;
    m_address.sin_addr.s_addr = INADDR_ANY;
    m_address.sin_port = htons(m_port);

    // Forcefully attaching socket to the port 8080
    if (bind(m_server_fd, (struct sockaddr*)&m_address,
        sizeof(m_address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(m_server_fd, BACKLOG) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return true;
}

/*
* Will accept a new connection by listening on it's address
*
*/

bool RPCServer::ListenForClient()
{

    int addrlen = sizeof(m_address);

    for (;;) // Endless loop. Probably good to have some type of controlled shutdown
    {
        if ((m_socket = accept(m_server_fd, (struct sockaddr*)&m_address,
            (socklen_t*)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Launch Thread to Process RPC
        // We will hold the thread ID into an array. Who know's we might want to join on them later

        pthread_t thread_id;
        printf("Launching Thread\n");
        int socket = m_socket;
        pthread_create(&thread_id, NULL, myThreadFun, (void*)&socket);
        // TODO Probably should save thread_id into some type of array
        //this->ProcessRPC();
    }
    return true;
}
















/*
* Going to populate a String vector with tokens extracted from the string the client sent.
* The delimter will be a ; 
* An example buffer could be "connect;mike;mike;"
*/
void RPCServer::ParseTokens(char * buffer, std::vector<std::string> & a)
{
    char* token;
    char* rest = (char *) buffer;

    while ((token = strtok_r(rest, ";", &rest)))
    {
        printf("%s\n", token);
        a.push_back(token);
    }

    return;
}


