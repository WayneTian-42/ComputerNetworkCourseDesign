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

typedef struct
{
    unsigned ID : 16;
    unsigned QR : 1;
    unsigned OPCODE : 4;
    unsigned AA : 1;
    unsigned TC : 1;
    unsigned RD : 1;
    unsigned RA : 1;
    unsigned Z : 3;
    unsigned RCODE : 4;
    unsigned QDCOUNT : 16;
    unsigned ANCOUNT : 16;
    unsigned NSCOUNT : 16;
    unsigned ARCOUNT : 16;

} HEADER;

extern int recordNum, debugLevel;
SOCKET sock;
SOCKADDR_IN clientAddr, serverAddr, tempAddr;
RECORD DNSrecord[1000];
USER userRord[65536];

char messageStructure[8];

int initSock(char *);
int readFile(char *);
void outputByBit(char);
void getHeader(HEADER *, char *);
int cmp(const void *, const void *);