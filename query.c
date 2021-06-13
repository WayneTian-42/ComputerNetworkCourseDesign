#include <stdio.h>
#include <string.h>
#include "query.h"
#include "request.h"

int changeID(char *message, unsigned char *oldID)
{
    unsigned short id;
    memcpy(&id, message, sizeof(id));
    unsigned short tmp;
    do
    {
        srand(time(NULL));
        tmp = rand() % 65536 + 1;
    } while (userRord[tmp].userAddr.sin_family);
    userRord[tmp].originalID = ntohs(id);
    memcpy(oldID, &id, 2);
    userRord[tmp].userAddr = tempAddr;
    return tmp;
}
void sendToServer(char *message, int length)
{
    unsigned char oldID[2];
    unsigned short newID = htons(changeID(message, oldID));
    memcpy(message, &newID, sizeof(newID));
    int sendLength = sendto(sock, message, length, 0, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
    if (sendLength < 0)
        printf("#Send to DNS Server Error %d\n", WSAGetLastError());
    else if (debugLevel == 2)
    {
        printf("SEND to %s:%d(%dbytes)  ", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), sendLength);
        printf("[ID ");
        printf("%x%x", (oldID[0] & 0xf0) >> 4, oldID[0] & 0x0f);
        printf("%x%x", (oldID[1] & 0xf0) >> 4, oldID[1] & 0x0f);
        printf("->");
        printf("%x%x", (message[0] & 0xf0) >> 4, message[0] & 0x0f);
        printf("%x%x]", (message[1] & 0xf0) >> 4, message[1] & 0x0f);
        printf("\n");
    }
}
void processMessage(char *message, HEADER *header)
{
    char start;                // 记录报文域名对应ip地址的偏移量
    unsigned short finalType;  // ip类型
    memcpy(&start, message + HEADERSIZE, sizeof(start));
    int off = start + HEADERSIZE + 1;
    while (start != 0)
    {
        memcpy(&start, message + off, sizeof(start));
        off += start + 1;
    }
    memcpy(&finalType, message + off + 1, sizeof(finalType));
    // finalType = ntohs(finalType);  // 为什么不需要转换字节序？
    // rocde不为0表示出错
    // ANCOUNT表示回答数，finalType表示类型，为1时表示TypeA即IPV4
    if (header->RCODE || !header->ANCOUNT)
        return;
    if (finalType == 1)
    {
        unsigned int pos;
        _Bool flg = FALSE;
        for (pos = 0; pos < recordNum; pos++)
        {
            if (DNSrecord[pos].ttl < difftime(time(NULL), DNSrecord[pos].recordTime))
            {
                flg = TRUE;
                break;
            }
        }
        if (!flg)
            recordNum++;
        if (pos >= 1000)
            return;
        clearRecord(pos);
        ipv4Message(message, pos, header->ANCOUNT, off + 4);  //加4是type和class字段，之后就是name（指针）
    }
    else
        return;
}

void ipv4Message(char *message, int pos, int anCount, int off)
{
    unsigned short type, class, dataLen;
    char iptmp[4] = {0}, ip[16] = {0};
    int count = 0;
    for (int i = 0; i < anCount && count < 20; i++, off += 12 + dataLen)
    {
        memcpy(&type, message + off + 2, sizeof(type));
        memcpy(&class, message + off + 4, sizeof(class));
        memcpy(&dataLen, message + off + 10, sizeof(dataLen));
        type = ntohs(type);
        class = ntohs(class);
        dataLen = ntohs(dataLen);
        if (type == 1 && class == 1)
        {
            unsigned int ttl;
            memcpy(&ttl, message + off + 6, sizeof(ttl));
            DNSrecord[pos].ttl = ntohl(ttl);

            memcpy(iptmp, message + off + 12, sizeof(iptmp));
            SOCKADDR_IN addrtmp;
            memset(&addrtmp, 0, sizeof(addrtmp));
            memcpy(&addrtmp.sin_addr.s_addr, iptmp, sizeof(iptmp));
            memcpy(ip, inet_ntoa(addrtmp.sin_addr), sizeof(ip));

            // DNSrecord[pos].ip[count] = (char *)malloc(sizeof(char) * (sizeof(ip) + 1));
            memcpy(DNSrecord[pos].ip[count], ip, strlen(ip));
            count++;
        }
        // if (debugLevel == 2) {}
    }
    if (count)
    {
        char domain[256];
        memset(domain, 0, sizeof(domain));
        getDomain(message + HEADERSIZE, domain);
        DNSrecord[pos].recordTime = time(NULL);
        // DNSrecord[pos].domain = (char *)malloc(sizeof(char) * (strlen(domain) + 1));
        memcpy(DNSrecord[pos].domain, domain, strlen(domain));
        if (pos >= recordNum)
            recordNum++;
        DNSrecord[pos].sum = count;
    }
    else
        clearRecord(pos);
    qsort(DNSrecord, recordNum, sizeof(RECORD), cmp);
    for (int i = recordNum - 1; i >= 0; i--)
    {
        if (strlen(DNSrecord[i].domain))
            break;
        else
            recordNum--;
    }
}
void snedAnswer(char *message, int length)
{
    unsigned short newID;
    // unsigned char new[2];
    memcpy(&newID, message, sizeof(newID));
    // memcpy(&new, message, sizeof(new));
    unsigned short oldID = htons(userRord[ntohs(newID)].originalID);
    memcpy(message, &oldID, sizeof(oldID));
    SOCKADDR_IN sourceAddr = userRord[ntohs(newID)].userAddr;
    memset(&userRord[ntohs(newID)], 0, sizeof(USER));

    int sendLength = sendto(sock, message, length, 0, (SOCKADDR *)&(sourceAddr), sizeof(sourceAddr));
    if (sendLength < 0)
        printf("#Send to Client Error %d\n", WSAGetLastError());
    else if (debugLevel > 1)
    {
        newID = ntohs(newID);
        printf("SEND to %s:%d(%dbytes)  ", inet_ntoa(sourceAddr.sin_addr), ntohs(sourceAddr.sin_port), sendLength);
        printf("[ID ");
        printf("%x%x", (newID & 0xf000) >> 12, (newID & 0x0f00) >> 8);
        printf("%x%x", (newID & 0x00f0) >> 4, newID & 0x000f);
        printf("->");
        printf("%x%x", (message[0] & 0xf0) >> 4, message[0] & 0x0f);
        printf("%x%x]", (message[1] & 0xf0) >> 4, message[1] & 0x0f);
        printf("\n");
    }
}
void clearRecord(int pos)
{
    /* if (DNSrecord[pos].domain != NULL)
    {
        free(DNSrecord[pos].domain);
        DNSrecord[pos].domain = NULL;
    } */
    memset(DNSrecord[pos].domain, 0, sizeof(DNSrecord[pos].domain));
    DNSrecord[pos].recordTime = 0;
    DNSrecord[pos].ttl = 0;
    for (int i = 0; i < DNSrecord[pos].sum; i++)
    {
        /* if (DNSrecord[pos].ip[i] != NULL)
        {
            free(DNSrecord[pos].ip[i]);
            DNSrecord[pos].ip[i] = NULL;
        } */
        memset(DNSrecord[pos].ip[i], 0, sizeof(DNSrecord[pos].ip[i]));
    }
    DNSrecord[pos].sum = 0;
}