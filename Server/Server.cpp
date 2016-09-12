#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>

#pragma comment(lib,"ws2_32.lib") //Winsock Library
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

SOCKET s;
struct sockaddr_in server, si_other;
int slen, recv_len;
char buf[BUFLEN];
WSADATA wsa;
std::vector<char*> bufVec;

std::mutex bufMutex;


void receiveMessage()
{
    bufMutex.lock();


    memset(buf, '\0', BUFLEN);
    if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
    {
        printf("recvfrom() failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
    printf("Data: %s\n", buf);

    bufVec.emplace_back(buf);

  
    bufMutex.unlock();
    
}

void returnMessage()
{
    bufMutex.lock();


    
    if (sendto(s, bufVec.front(), recv_len, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
    {
        printf("sendto() failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    bufVec.erase(bufVec.begin());


    bufMutex.unlock();
}

int main()
{
    

    slen = sizeof(si_other);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("Waiting for data..."); fflush(stdout);

        std::thread receiveThread(receiveMessage);        

        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::thread returnThread(returnMessage);

        returnThread.join();
        receiveThread.join();
    }

    

    closesocket(s);
    WSACleanup();

    return 0;
}
