#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <time.h>
#include "init.h"

int initSock(char *DNSServer)
{
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

    return 1;
}
int readFile(char *localFile)
{
    FILE *fp = NULL;
    if ((fp = fopen(localFile, "r")) == NULL)
    {
        printf("Open file Error!\n");
        return -1;
    }

    int num = 0, flg = 0;
    char buffer[1024];
    while (fscanf(fp, "%s", buffer) != EOF)
    {
        int size = strlen(buffer);
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
        }
        else  //记录域名
        {
            DNSrecord[num].domain = (char *)malloc(size + 1);
            strcpy(DNSrecord[num].domain, buffer);
            flg = 0;
            num++;
        }
    }
    fclose(fp);
    //快排，变为有序序列，便于查找
    qsort(DNSrecord, num, sizeof(RECORD), cmp);
    return num;
}
//先这么改，不行就修改二分算法
int cmp(const void *a, const void *b)
{
    if (((RECORD *)a)->domain != NULL && ((RECORD *)b)->domain != NULL)
        return strcmp(((RECORD *)a)->domain, ((RECORD *)b)->domain);
    else if (!((RECORD *)a)->domain)
        return FALSE;
    else
        return FALSE;
}