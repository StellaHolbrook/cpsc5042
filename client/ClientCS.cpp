// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <vector>
#include <iterator>
#include <iostream>

using namespace std;
// This is a C Program. No classes. You may turn this into an Object Oriented C++ program if you wish

void ParseTokens(char* buffer, std::vector<std::string>& a)
{
    char* token;
    char* rest = (char*)buffer;

    while ((token = strtok_r(rest, ";", &rest)))
    {
        a.push_back(token);
    }

    return;
}

/*
    ConnectToServer will connect to the Server based on command line
*/
bool ConnectToServer(const char *serverAddress, int port, int & sock)
{
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, serverAddress, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return false;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return false;
    }

    return true;
}

/*
    Connect will prompt the user to enter the username and password, and check if login successfully
*/
bool Connect(int sock) {
    char buffer[1024] = { 0 };
    const char* RPCcall = "connect;";
    char username[20];
    char password[20];


    // Prompt the user to enter the username and password
    cout << "\033[;34m\nEnter username: \033[0m";
    scanf("%s", username);
    cout << "\033[;34mEnter password: \033[0m";
    scanf("%s", password);

    strcpy(buffer, RPCcall);
    strcat(buffer, username);
    strcat(buffer, ";");
    strcat(buffer, password);
    strcat(buffer, ";");

    int nlen = strlen(buffer);
    buffer[nlen] = 0;   // Put the null terminator
    int valwrite = send(sock, buffer, strlen(buffer)+1, 0);
    int valread = read(sock, buffer, 1024);

    // Display the login status and whether login successfully or not
    printf("\nLogin status=%s\n", buffer);
    if(strcmp(buffer, "1") == 0){
        cout << "Login successfully\n\n" ;
        cout << "\033[1;33m***** Welcome to the quiz game! *****\033[0m\n\n";
        cout << "\033[;33mThere will be 10 quiz questions about computer science. \nEach question have four options, enter you answer with A, B, C or D.\nWhen you answer is correct, you will win 1 point. \nAre you ready?\033[0m\n\n";
        cout << "\033[1;33mLet's play!\033[0m\n\n";

        return true;
    }
    else{
        printf("Login failed\n");
        return false;
    }
}

/*
    GetQuestion will send getquestion RPC to the server and receive the quiz question from the server
*/
void GetQuestion(int sock, int num, vector<std::string>& arrayTokens) {
    char buffer[1024] = { 0 };
    const char* RPCcall = "getquestion;";

    strcpy(buffer, RPCcall);
    string count = std::to_string(num);
    const char* val = count.c_str();
    strcat(buffer, val);
    strcat(buffer, ";");

    int nlen = strlen(buffer);
    buffer[nlen] = 0;   // Put the null terminator
    int valwrite = send(sock, buffer, strlen(buffer) + 1, 0);   
    int valread = read(sock, buffer, 1024);

    arrayTokens.clear();
    ParseTokens(buffer, arrayTokens);
    string question = arrayTokens[0];
    string answerA = arrayTokens[1];
    string answerB = arrayTokens[2];
    string answerC = arrayTokens[3];
    string answerD = arrayTokens[4];

    // Display the quiz question
    cout << endl << "\033[;35m" << question << "\033[0m\n";
    cout << answerA << endl;
    cout << answerB << endl;
    cout << answerC << endl;
    cout << answerD << endl << endl;
}

/*
    PostAnswer will prompt the user to enter the answer, and receive whether the answer is correct or not from the server
*/
void PostAnswer(int sock, int num) {
    char buffer[1024] = { 0 };
    const char* RPCcall = "postanswer;";
    char answer[20];

    // Prompt the user to enter the answer
    cout << "\033[;35mEnter your answer: \033[0m";
    scanf("%s", answer);

    strcpy(buffer, RPCcall);
    string count = std::to_string(num);
    const char* val = count.c_str();
    strcat(buffer, val);
    strcat(buffer, ";");

    strcat(buffer, answer);
    strcat(buffer, ";");

    int nlen = strlen(buffer);
    buffer[nlen] = 0;   // Put the null terminator
    int valwrite = send(sock, buffer, strlen(buffer) + 1, 0);   
    int valread = read(sock, buffer, 1024);

    // Display whether the answer is correct or not
    cout << "\n\033[;32m" << buffer << "\033[0m";
}

/*
    Get Score will call the update score RPC that will send the current amount
    of questions the user has answered correctly.
 */
void GetScore(int sock, int num){
    char buffer [1024] = { 0 };
    const char * RPCcall = "getscore";
    strcpy(buffer, RPCcall);
    strcat(buffer, ";");

    std::string count = std::to_string(num);
    const char* val = count.c_str();
    strcat(buffer, val);
    strcat(buffer, ";");

    int nlen = strlen(buffer);
    buffer[nlen] = 0;   // Put the null terminator
    int valwrite = send(sock, buffer, strlen(buffer) + 1, 0);
    int valread = read(sock, buffer, 1024);

    // Display the score
    cout << "\033[;32m" << buffer << "\033[0m";
}

/*
    Disconnect will disconnect from the server
*/
void Disconnect(int sock)
{
    char buffer[1024] = { 0 };

    strcpy(buffer, "disconnect;");
    int nlen = strlen(buffer);
    buffer[nlen] = 0;   // Put the null terminator
    int valwrite = send(sock, buffer, strlen(buffer) + 1, 0);
    int valread = read(sock, buffer, 1024);

    // Display disconnect status and whether its successfully or not
    printf("\nDisconnnect status=%s\n", buffer);
    if(strcmp(buffer, "1") == 0){
        printf("Disconnect successfully\n\n");
    }
    else{
        printf("Disconnect failed\n\n");
    }
}

int main(int argc, char const* argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    const char *serverAddress = argv[1];
    const int port = atoi(argv[2]);
    const int numQuestions = 10;
    vector<std::string> arrayTokens;

    bool bConnect = ConnectToServer(serverAddress, port, sock);

    if (bConnect == true)
    {
        bool login = Connect(sock);

        // If login successfully will start quiz
        if(login == true) {
            for (int i = 0; i < numQuestions; i++) {
                GetQuestion(sock, i, arrayTokens);
                PostAnswer(sock, i);
                GetScore(sock, i);
            }
        }
    }
    else
    {
        printf("Exit without calling RPC");
    }

    // Do a Disconnect Message
    if (bConnect == true)
    {
        Disconnect(sock);
    }
    else
    {
        printf("Exit without calling RPC");
    }

    // Terminate connection
    close(sock);

    return 0;
}
