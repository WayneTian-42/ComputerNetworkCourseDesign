#include <string.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "init.h"

int recordNum = 0, debugLevel = 0;

int initSock(char *DNSServer)
{
    printf("Bind UDP port 53 ");
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)  //
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return -1;
    }

    //创建一个socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
        return -1;
    }

    //创建客户端地址
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(PORT);

    //创建DNS服务器地址
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(DNSServer);  //通过ipconfig/all查询
    serverAddr.sin_port = htons(PORT);

    //绑定sock以及对应地址
    if (bind(sock, (SOCKADDR *)&clientAddr, sizeof(clientAddr)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return -1;
    }
    else
        printf("...OK!\n");

    return 1;
}
int readFile(char *localFile)
{
    printf("Try to load loacl file \" %s \" ", localFile);
    FILE *fp = NULL;
    if ((fp = fopen(localFile, "r")) == NULL)
    {
        printf("Open file Error!\n");
        return 0;
    }
    printf("...Ok\n");
    int num = 0, flg = 0, memory = 0;
    char buffer[1024];
    while (fscanf(fp, "%s", buffer) != EOF)
    {
        int size = strlen(buffer);
        memory += size;
        /* if (buffer[0] < '0' || buffer[0] > '9')
            continue; */
        if (!flg)  //记录ip地址
        {
            DNSrecord[num].ip[0] = (char *)malloc(size + 1);
            DNSrecord[num].sum = 1;
            DNSrecord[num].recordTime = time(NULL);
            DNSrecord[num].ttl = 10;  //默认为一小时
            strcpy(DNSrecord[num].ip[0], buffer);
            flg = 1;
            if (debugLevel == 2)
            {
                printf("%8d: ", num + 1);
                printf("%-20s", DNSrecord[num].ip[0]);
            }
        }
        else  //记录域名
        {
            DNSrecord[num].domain = (char *)malloc(size + 1);
            strcpy(DNSrecord[num].domain, buffer);
            flg = 0;
            if (debugLevel == 2)
                printf("%s\n", DNSrecord[num].domain);
            num++;
        }
    }
    fclose(fp);
    printf("%d domains, occupy %d bytes memory\n", num, memory);
    //快排，变为有序序列，便于查找
    qsort(DNSrecord, num, sizeof(RECORD), cmp);
    return num;
}
void outputByBit(char c)
{
    unsigned char tmp = c;
    printf("%x%x  ", (tmp & 0xf0) >> 4, tmp & 0x0f);
}
void getHeader(HEADER *header, char *message)
{
    unsigned short tmp;
    memcpy(&tmp, message, sizeof(tmp));
    header->ID = ntohs(tmp);
    memcpy(&tmp, message + 2, sizeof(tmp));
    header->QR = (ntohs(tmp) & 0x8000) >> 15;
    tmp <<= 1;
    header->OPCODE = (ntohs(tmp) & 0xf000) >> 12;
    tmp <<= 4;
    header->AA = (ntohs(tmp) & 0x8000) >> 15;
    tmp <<= 1;
    header->TC = (ntohs(tmp) & 0x8000) >> 15;
    tmp <<= 1;
    header->RD = (ntohs(tmp) & 0x8000) >> 15;
    tmp <<= 1;
    header->RA = (ntohs(tmp) & 0x8000) >> 15;
    tmp <<= 1;
    header->Z = (ntohs(tmp) & 0x7000) >> 13;
    tmp <<= 4;
    header->RCODE = (ntohs(tmp) & 0xf000) >> 12;
    memcpy(&tmp, message + 4, sizeof(tmp));
    header->QDCOUNT = ntohs(tmp);
    memcpy(&tmp, message + 6, sizeof(tmp));
    header->ANCOUNT = ntohs(tmp);
    memcpy(&tmp, message + 8, sizeof(tmp));
    header->NSCOUNT = ntohs(tmp);
    memcpy(&tmp, message + 10, sizeof(tmp));
    header->ARCOUNT = ntohs(tmp);
}
//先这么改，不行就修改二分算法
int cmp(const void *a, const void *b)
{
    if (((RECORD *)a)->domain != NULL && ((RECORD *)b)->domain != NULL)
        return strcmp(((RECORD *)a)->domain, ((RECORD *)b)->domain);
    else if (((RECORD *)a)->domain == NULL)
        return 1;
    else
        return 1;
}