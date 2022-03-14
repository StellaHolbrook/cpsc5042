#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <vector>
#include <iterator>
#include <fstream>
#include <array>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

#include "RPCImpl.h"
#include "LocalContext.h"

//#define PORT 8081

using namespace std;

pthread_mutex_t lock;

typedef struct _GlobalContext {
    int score = 0;
} GlobalContext;

GlobalContext topScore; // We need to protect this, as we don't want bad data

typedef struct _Question {
    std::string body;
    std::string answerA;
    std::string answerB;
    std::string answerC;
    std::string answerD;
    std::string correctAnswer;
} Question;

Question currentQuestion; 

// create an array of these structres
struct _Question questionArray[10];

// A normal C function that is executed as a thread 
// when its name is specified in pthread_create()
void* threadFun(void* vargp)
{
    sleep(1);
    pthread_mutex_lock(&lock);

    int newScore = *(int *) vargp;
    printf("\nPrinting topScore from Thread \n");
    topScore.score = newScore;
    printf("topScore: %d\n", topScore.score);
    printf("Done with Thread\n");

    pthread_mutex_unlock(&lock);

    return NULL;

}

RPCImpl::RPCImpl(int socket)
{
    m_socket = socket;
    m_rpcCount = 0;
    readFile(questionArray);
    currentSocre = LocalContext();
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
        a.push_back(token);
    }

    return;
}

// once this gets called, array collection will have values from text file
void RPCImpl::readFile(struct _Question questionArray[])
{
    ifstream fin;
    fin.open ("Questions.txt");
    if (!fin.fail())
    {
        // Read file of 10 questions
        for (int i = 0; i < numOfQuestions; i++)
        {
            string body, answerA, answerB, answerC, answerD, correctAnswer;

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
            questionArray[i].correctAnswer = correctAnswer;
        }
        fin.close();
    }
}

/*
* ProcessRPC will examine buffer and will essentially control
*/
bool RPCImpl::ProcessRPC()
{
    const char* rpcs[] = { "connect", "disconnect", "getquestion", "postanswer", "getscore"};
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
        printf("\nrpc=%s\n\n", buffer);

        arrayTokens.clear();
        this->ParseTokens(buffer, arrayTokens);

        // Enumerate through the tokens. The first token is always the specific RPC
        for (vector<string>::iterator t = arrayTokens.begin(); t != arrayTokens.end(); ++t)
        {
            printf("Debugging our tokens\n");
            cout << "token =" << *t << endl;
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

        else if ((bConnected == true) && (aString == "getquestion"))
        {
            bStatusOk = ProcessGetQuestionRPC(arrayTokens);   // GetQuestion RPC
        }

        else if ((bConnected == true) && (aString == "postanswer"))
        {
            bStatusOk = ProcessPostAnswerRPC(arrayTokens);   // PostAnswer RPC
        }
        else if ((bConnected == true) && (aString == "getscore"))
        {
            bStatusOk = ProcessGetScoreRPC(arrayTokens);   // GetScore RPC
        }
        else
        {
            printf("Invalid rpc\n");
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
    if ((userNameString == "group1") && (passwordString == "group1pass"))
    {
        strcpy(szBuffer, "1"); // Connected
        printf("User login successfully\n");
    }
    else
    {
        strcpy(szBuffer, "0"); // Not Connected
        printf("User login failed\n");
    }

    // Send Response back on our socket
    int nlen = strlen(szBuffer);
    szBuffer[nlen] = 0;
    send(this->m_socket, szBuffer, strlen(szBuffer) + 1, 0);

    return true;
}

/*
    ProcessGetQuestionRPC will send the quiz question to the client
*/
bool RPCImpl::ProcessGetQuestionRPC(std::vector<std::string>& arrayTokens)
{
    char buffer[1024] = { 0 };
    string str = arrayTokens[1];
    int num = std::stoi(str);
    string question = questionArray[num].body;
    string answerA = questionArray[num].answerA;
    string answerB = questionArray[num].answerB;
    string answerC = questionArray[num].answerC;
    string answerD = questionArray[num].answerD;

    strcpy(buffer, const_cast<char*>(question.c_str()));
    strcat(buffer, ";");
    strcat(buffer, const_cast<char*>(answerA.c_str()));
    strcat(buffer, ";");
    strcat(buffer, const_cast<char*>(answerB.c_str()));
    strcat(buffer, ";");
    strcat(buffer, const_cast<char*>(answerC.c_str()));
    strcat(buffer, ";");
    strcat(buffer, const_cast<char*>(answerD.c_str()));
    strcat(buffer, ";");

    // Send Response back on our socket
    int nlen = strlen(buffer);
    buffer[nlen] = 0;
    send(this->m_socket, buffer, strlen(buffer) + 1, 0);

    printf("Send question successfully\n");

    return true;
}

/*
    PostAnswerRPC will check and send whether the answer is correct or not to the client
*/
bool RPCImpl::ProcessPostAnswerRPC(std::vector<std::string>& arrayTokens)
{
    string numString = arrayTokens[1];
    string userAnswerString = arrayTokens[2];
    const char* userAnswer = userAnswerString.c_str();
    int num = std::stoi(numString);
    string correctAnswerString = questionArray[num].correctAnswer;
    const char* correctAnswer = correctAnswerString.c_str();
    char buffer[1024] = { 0 };

    if(strcasecmp(userAnswer, correctAnswer) == 0)
    {
        strcpy(buffer, "Your answer is correct");
        currentSocre.updateScore();
    } 
    else
    {
        strcpy(buffer, "Your answer is wrong. The correct answer is ");
        strcat(buffer, const_cast<char*>(correctAnswer));
    }

    // Send Response back on our socket
    int nlen = strlen(buffer);
    buffer[nlen] = 0;
    send(this->m_socket, buffer, strlen(buffer) + 1, 0);

    printf("Post answer successfully\n");

    return true;
}

bool RPCImpl::ProcessGetScoreRPC(std::vector<std::string>& arrayTokens) {
    string numString = arrayTokens[1];
    int num = stoi(numString);
    char buffer[1024] = { 0 };
    //Convert final score to char *

    string score = to_string(currentSocre.getCurrentScore());
    const char * s = score.c_str();

    if(num == numOfQuestions - 1){
        // Check whether the score of current player is higher than the score of history
        // if yes, update the score which stores in global context 
        if(currentSocre.getCurrentScore() > topScore.score){
            pthread_t thread_id;
            pthread_mutex_init(&lock, NULL);

            int score = currentSocre.getCurrentScore();

            if( pthread_create(&thread_id, NULL, threadFun, (void*)&score) != 0 )
                printf("Failed to create thread\n");

            pthread_join(thread_id, NULL);
        }

        strcpy(buffer, "\n\n\n\033[;33mThanks for taking the quiz.\nYou finished with "
                       "a final score of ");
        strcat(buffer, s);
        strcat(buffer, " out of ");
        //Convert length of qs to final score.
        string size = to_string(numOfQuestions);
        const char * sz = size.c_str();
        string scoreString = to_string(topScore.score);
        const char * highScore = scoreString.c_str();
        strcat(buffer, sz);
        strcat(buffer, " correct. \nThe highest score in history is: ");
        strcat(buffer, highScore);
        strcat(buffer, ".\n\nPlease play again soon!\033[0m\n");
    }
    else {
        strcpy(buffer, "\nYou currently have ");
        strcat(buffer, s);
        strcat(buffer, " correct.\n");

    }
    // Send Response back on our socket
    int nlen = strlen(buffer);
    buffer[nlen] = 0;
    send(this->m_socket, buffer, strlen(buffer) + 1, 0);

    printf("Post answer successfully\n");

    return true;

}

/*
*/
bool RPCImpl::ProcessDisconnectRPC()
{
    char szBuffer[16];
    strcpy(szBuffer, "1");

    printf("User disconnect successfully\n");

    // Send Response back on our socket
    int nlen = strlen(szBuffer);
    szBuffer[nlen] = 0;
    send(this->m_socket, szBuffer, strlen(szBuffer) + 1, 0);
    return true;
}