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
#include <cassert>


   #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
#define INDIVIDUAL_REFRESH_RATE -1
#define CONNECTION_REFRESH_RATE 10
#define SEND_DELAY 10
#define SHOW_STATS_AT 10
#define DEBUG_ENABLED false

#define PACKET_SCRIPT_SERVER "packetscript_server.lua"
#define PACKET_SCRIPT_CLIENT "packetscript_client.lua"
unsigned int g_uiClientsConnected;
unsigned int g_uiPacketsReceived;
unsigned int g_uiPacketsSent;
unsigned int g_uiPacketsProcessed;
unsigned int g_uiThreadsRunning;
unsigned int g_pot;
/*Kevin's C10K problem implentation*/
using namespace std;
/*
struct LMSPacket{
    int fromSocket;
    int intValue;
    bool boolValue;
    std::string stringValue;
    int responseCode;
};
*/
struct LMSPacket{
    int randomNumber;
};
struct SIndividualSocket{
    unsigned int uiSocket;
    pthread_t pthThread;
};

void debug_msg(std::string message){
    if(DEBUG_ENABLED) std::cout << message << std::endl;
}
bool imitate_process(char input[50], unsigned int uiSendTo){

}
static int l_remove_from_pot(lua_State *L){
        if(lua_isnumber(L, -1)){
        int randomNumber =  lua_tonumber(L, -1);
        g_pot -= randomNumber;
          g_uiPacketsProcessed++;
        }
}
static int l_add_to_pot(lua_State *L){
        if(lua_isnumber(L, -1)){
        int randomNumber =  lua_tonumber(L, -1);
        g_pot += randomNumber;
        g_uiPacketsProcessed++;
        }
        return 1;
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
            LMSPacket incomingPacket;
            if((recv(pollSocketIndividual.fd, (char*)&incomingPacket, sizeof(incomingPacket),0)) != 0){

                debug_msg("Received packet");
                lua_State *L = luaL_newstate();
                luaL_openlibs(L);
                luaL_dofile(L, PACKET_SCRIPT_SERVER);
                lua_getglobal(L,"respond");
                assert(lua_isfunction(L, -1));

                lua_pushnumber(L, incomingPacket.randomNumber);
                lua_pushcfunction(L, l_add_to_pot);
                lua_setglobal(L, "add_to_pot");

                lua_pushcfunction(L, l_remove_from_pot);
                lua_setglobal(L,"remove_from_pot");
                if(lua_pcall(L, 1, 0, 0) != 0){
                }
                debug_msg("Lua handling closed for packet receive");
                /*
                lua_newtable(L);
                lua_pushnumber(L, incomingPacket.intValue);
                lua_setfield(L, -2, "intValue");
                lua_pushnumber(L,  incomingPacket.responseCode);
                lua_setfield(L, -2, "responseCode");
                lua_pushnumber(L,  incomingPacket.boolValue);
                lua_setfield(L, -2, "boolValue");
                lua_pushstring(L,  incomingPacket.stringValue.c_str());
               lua_setfield(L, -2, "stringValue");
                lua_pushnumber(L,  incomingPacket.fromSocket);
                lua_setfield(L, -2, "fromSocket");


                if(lua_pcall(L, 1, 0, 0) != 0){
                }
                */
                lua_close(L);
                g_uiPacketsReceived++;
                /*
                struct LMSPacket{
    int fromSocket;
    int intValue;
    bool boolValue;
    std::string stringValue;
    int responseCode;
    bool doRespond;
};
*/
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
    g_pot = 0;

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
            std::cout << "Pot: "<< g_pot << std::endl;
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
