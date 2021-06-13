#include <string.h>
/* #define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS */
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

#include <stdio.h>
#include "init.h"

int recordNum = 0, debugLevel = 0, ban = 0;

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
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
        return -1;
    }

    BOOL bEnalbeConnRestError = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(sock, SIO_UDP_CONNRESET, &bEnalbeConnRestError, sizeof(bEnalbeConnRestError), NULL, 0, &dwBytesReturned, NULL, NULL);
    //创建客户端地址
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(PORT);

    //创建DNS服务器地址
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(DNSServer);  //通过ipconfig /all查询
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
            // DNSrecord[num].ip[0] = (char *)malloc(sizeof(char) * (size + 1));
            DNSrecord[num].sum = 1;
            DNSrecord[num].recordTime = time(NULL);
            DNSrecord[num].ttl = 120;  //默认为2分钟
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
            // DNSrecord[num].domain = (char *)malloc(sizeof(char) * (size + 1));
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
    printf("%x%x ", (tmp & 0xf0) >> 4, tmp & 0x0f);
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
void getDomain(char *message, char *domain)
{
    // nextLength表示下一段域名长度
    int nextLength = message[0];
    // totalLength表示到现在为止总长度
    int totalLength = 0;

    while (1)
    {
        int i = 1;
        for (; i <= nextLength; i++)
        {
            if (message[i + totalLength] <= 'Z' && message[i + totalLength] >= 'A')
                domain[i + totalLength - 1] = message[i + totalLength] + 32;
            else
                domain[i + totalLength - 1] = message[i + totalLength];
        }
        // tmp记录next，因为马上next要更新
        int tmp = nextLength;
        nextLength = message[totalLength + nextLength + 1];
        //域名以00结尾，非0就可以在输出时用.来代替
        if (nextLength)
            domain[i + totalLength - 1] = '.';
        else
            break;
        totalLength += tmp + 1;
    }
}

int cmp(const void *a, const void *b)
{
    if (strlen(((RECORD *)a)->domain) && strlen(((RECORD *)b)->domain))
        return strcmp(((RECORD *)a)->domain, ((RECORD *)b)->domain);
    else if (!strlen(((RECORD *)a)->domain))
        return 0;
    else
        return 1;
}