#pragma once
#include <WinSock2.h>
#include <string.h>
// #include <winsock2.h>
#include <time.h>

#define PORT 53
#define MESSAGESIZE 1024
#define HEADERSIZE 12

//查询记录
typedef struct
{
    char *domain;
    char *ip[20];
    int sum, type;
    time_t recordTime, ttl;
} RECORD;

//哈希表记录用户地址与IP
typedef struct
{
    SOCKADDR_IN userAddr;
    unsigned short originalID;
} USER;

int recordNum;
SOCKET sock;
SOCKADDR_IN clientAddr, serverAddr, tempAddr;
RECORD DNSrecord[1000];
USER userRord[65536];

int initSock(char *);
int readFile(char *);
int cmp(const void *, const void *);