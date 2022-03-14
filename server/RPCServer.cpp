#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <vector>
#include <iterator>
#include <pthread.h>
#include <semaphore.h>

#include "RPCServer.h"
#include "RPCImpl.h"

//#define PORT 8081

using namespace std;

pthread_mutex_t mutex;
sem_t full;
sem_t empty;

// A normal C function that is executed as a thread 
// when its name is specified in pthread_create()
void* myThreadFun(void* vargp)
{

    sleep(1);
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);

    int socket = *(int *) vargp;
    printf("Printing from Thread for each player\n");
    RPCImpl *rpcImplObj = new RPCImpl(socket);
    rpcImplObj->ProcessRPC();   // This will go until client disconnects;
    printf("Done with Thread");

    pthread_mutex_unlock(&mutex);
    sem_post(&full);

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
    if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    printf("Got socket\nAbout to bind\n");

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
        pthread_t thread_id[MaxPlayerNumber];
        printf("Launching Thread\n");
        int socket = m_socket;

        pthread_mutex_init(&mutex, NULL);
        sem_init(&empty, 0, MaxPlayerNumber);
        sem_init(&full, 0, 0);

        printf("playerID=%d\n", playerID);
        
        if( pthread_create(&thread_id[playerID++], NULL, myThreadFun, (void*)&socket) != 0 )
            printf("Failed to create thread\n");

        if( playerID >= MaxPlayerNumber)
        {
            playerID = 0;
            while(playerID < MaxPlayerNumber)
            {
                pthread_join(thread_id[playerID++],NULL);
            }
            playerID = 0;
        }

        pthread_mutex_destroy(&mutex);
        sem_destroy(&empty);
        sem_destroy(&full);

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


