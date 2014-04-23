#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string>
#include <poll.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <cstring>
#include <cstdio>

#define INDIVIDUAL_REFRESH_RATE -1
#define CONNECTION_REFRESH_RATE 10
#define SEND_DELAY 10
#define SHOW_STATS_AT 50
#define DEBUG_ENABLED false
unsigned int g_uiClientsConnected;
unsigned int g_uiPacketsReceived;
unsigned int g_uiPacketsSent;
unsigned int g_uiPacketsProcessed;
unsigned int g_uiThreadsRunning;
/*Kevin's C10K problem implentation*/
using namespace std;
struct SIndividualSocket{
    unsigned int uiSocket;
    pthread_t pthThread;
};

void debug_msg(std::string message){
    if(DEBUG_ENABLED) std::cout << message << std::endl;
}
bool imitate_process(char input[50], unsigned int uiSendTo){
    if(!strcmp(input, "Compare One")){
    }
    if(!strcmp(input, "Compare Two")){
    }
    if(!strcmp(input, "Compare Three")){
    }
    debug_msg("Processed packet");
    g_uiPacketsProcessed++;
    char* test = "Yes";
    send(uiSendTo, test, 3, 0);
    debug_msg("Sent packet");
    g_uiPacketsSent++;
}
void* thread_individual(void* args){
    debug_msg("New thread was created for client");
    SIndividualSocket* myIndividual = (SIndividualSocket*) args;
    bool bClientConnected = true;
    pollfd pollSocketIndividual;
    pollSocketIndividual.fd = myIndividual->uiSocket;
    pollSocketIndividual.events = POLLIN;
    int iPollActivityIndividual;
    while(bClientConnected){
        iPollActivityIndividual = poll(&pollSocketIndividual, 1, INDIVIDUAL_REFRESH_RATE);
        if(iPollActivityIndividual < 0) perror("poll_individual");
        if(pollSocketIndividual.revents & POLLIN){
            char buffer[50];
            if((recv(pollSocketIndividual.fd, buffer, 49,0)) != 0){
                debug_msg("Received packet");
                g_uiPacketsReceived++;
                imitate_process(buffer, pollSocketIndividual.fd);
            }else{
                debug_msg("Client has disconnected");
                g_uiClientsConnected--;
                bClientConnected = false;
            }
        }
    }
}

void* thread_connection(void*){
    bool bServerOnline = true;
    g_uiClientsConnected = 0;
    g_uiPacketsReceived = 0;
    g_uiPacketsSent = 0;
    g_uiPacketsProcessed = 0;
    g_uiThreadsRunning = 0;
    struct sockaddr_in sMasterAddress;
    sMasterAddress.sin_port = htons(6666);
    sMasterAddress.sin_addr.s_addr = INADDR_ANY;
    sMasterAddress.sin_family = AF_INET;

    unsigned int uiMasterSocket;
    uiMasterSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(bind(uiMasterSocket, (struct sockaddr*)&sMasterAddress, sizeof sMasterAddress) < 0){
        perror("bind");
        bServerOnline = false;
    }
    if(listen(uiMasterSocket, 0) < 0){
        perror("listen");
        bServerOnline = false;
    }


    int iPollActivity;
    pollfd pollMasterSocket;
    pollMasterSocket.fd = uiMasterSocket;
    pollMasterSocket.events = POLLIN;
    bool bDisplayToggle = true;
    unsigned int uiEventsFired = 1;
    if(bServerOnline) std::cout << "Ready to accept connections" << std::endl;
    while(bServerOnline){

        iPollActivity = poll(&pollMasterSocket, 1, CONNECTION_REFRESH_RATE);
        if(iPollActivity < 0)  perror("poll");
        if(pollMasterSocket.revents & POLLIN){
            debug_msg("Client has connected");
            if(unsigned int uiNewSocket = accept(pollMasterSocket.fd, NULL, NULL)){
            SIndividualSocket sNewIndividual;
            sNewIndividual.uiSocket = uiNewSocket;
            pthread_t pthNewThread;
            sNewIndividual.pthThread = pthNewThread;
            pthread_create(&pthNewThread, NULL, thread_individual, (SIndividualSocket*)&sNewIndividual);
            pthread_tryjoin_np(pthNewThread, NULL);
            g_uiClientsConnected++;
            g_uiThreadsRunning++;
            uiEventsFired++;
            }else{
            perror("accept");
            }

        }
        if(uiEventsFired % (SHOW_STATS_AT+1) == 0) {
            std::cout << "Clients connected: " << g_uiClientsConnected << std::endl;
             std::cout << "Threads Running: " << g_uiThreadsRunning << std::endl;
            std::cout << "Packets sent: " << g_uiPacketsSent << std::endl;
            std::cout << "Packets received: " << g_uiPacketsReceived << std::endl;
            std::cout << "Packets processed: "<< g_uiPacketsProcessed << std::endl;
            std::cout << std::endl;
            uiEventsFired = 1;
        }
    }
    std::cout << "Session has finished" << std::endl;
}

int main()
{
    pthread_t pthConnection;
    pthread_create(&pthConnection, NULL, thread_connection, NULL);
    pthread_join(pthConnection, NULL);
    cout << "Hello world!" << endl;
    return 0;
}
