#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>

#include "httpUtils.h"

typedef struct client_info
{
    pthread_t thread_id;
    int clientsock;
    int index;
    const char *ip;
    int port;
} Client_info;

Client_info clients[MAXPENDING];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *functionClient(void* client)
{
    int sock = ((Client_info *)client)->clientsock;
    const char *ip = ((Client_info *)client)->ip;
    int index = ((Client_info *)client)->index;

    char* dataBuffer=(char*)malloc(BUFFSIZE*sizeof(char));
    std::string clientMessage;

    while(1)
    {
        int n;
        if((n=recv(sock,dataBuffer,BUFFSIZE,0))==-1){         
            die("ERROR receiving Client HTTP request");
        }
        else{
            dataBuffer[n]=0x00;
        }

        clientMessage=std::string(dataBuffer);
        if(clientMessage.find("\r\n\r\n")!=-1)
        {
            std::vector<std::string> lines=split(clientMessage,'\n');
            for(auto line:lines)
            {
                line=clientMessage.substr(0,clientMessage.find("\r\n"));
                std::string do_way=split(line,' ')[0];
                if("GET"==do_way)
                {
                    do_GET(sock,split(line,' ')[1]);
                }
                else if("POST"==do_way)
                {
                    //Get the data section
                    std::string dataPart=clientMessage.substr(clientMessage.rfind("\r\n\r\n"));

                    std::string login=dataPart.substr(dataPart.find("=")+1,dataPart.find("&")-dataPart.find("=")-1);
                    std::string pwd=dataPart.substr(dataPart.rfind("=")+1);
                    do_POST(sock,login,pwd);
                }
                else
                {
                    error(sock);
                }
            }  
        }
        else
        {
            //DO NOTHING
        }
    }
    free(dataBuffer);
    close(sock);
    pthread_mutex_lock(&clients_mutex);
    clients[index].index = -1;
    pthread_mutex_unlock(&clients_mutex);
    return NULL;
}


void init()
{
    for (int i = 0; i < MAXPENDING; i++)
    {
        clients[i].index = -1;
    }
    fprintf(stdout, "initialize success\n");
}


int main(int argc, char *argv[])
{
    fprintf(stdout,"Initializing..\n");
    init();
    int serversock, clientsock;
    struct sockaddr_in echoserver, echoclient;

    /* Create the TCP socket */
    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        die("Failed to create socket");
    }
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));     /* Clear struct */
    echoserver.sin_family = AF_INET;                /* Internet/IP */
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY); /* Incoming addr */
    echoserver.sin_port = htons(PORT);              /* server port */
    /* Bind the server socket */
    if (bind(serversock, (struct sockaddr *)&echoserver, sizeof(echoserver)) < 0)
    {
        die("Failed to bind the server socket");
    }
    /* Listen on the server socket */
    if (listen(serversock, MAXPENDING) < 0)
    {
        die("Failed to listen on server socket");
    }
    fprintf(stdout,"Server listening at PORT %d...\n",PORT);

    /* Run until cancelled */
    while (1)
    {
        unsigned int clientlen = sizeof(echoclient);
        if ((clientsock = accept(serversock, (struct sockaddr *)&echoclient,&clientlen)) < 0)
        {
            die("Failed to accept client connection");
        }

        const char *ip = inet_ntoa(echoclient.sin_addr);
        int port = echoclient.sin_port;

        int index = -1;

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAXPENDING; i++)
        {
            if (clients[i].index == -1)
            {
                clients[i].index = i;
                index = i;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (index == -1)
        {
            //TODO: decide protocal to refuse connection
            fprintf(stdout, "MAX PENDING");
            continue;
        }

        clients[index].clientsock = clientsock;
        clients[index].ip = ip;
        clients[index].port = port;

        fprintf(stdout, "Client connected: %s %d\n", ip, port);

        if (pthread_create(&(clients[index].thread_id), NULL, functionClient, &clients[index]) != 0)
        {
            die("pthread_create() error");
        }
        pthread_detach(clients[index].thread_id);  
    }
}