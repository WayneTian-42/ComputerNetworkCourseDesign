#include <stdio.h>
#include <string.h>
#include "request.h"

int searchLocal(char *domain, int num)
{
    int left = 0, right = num - 1;
    //二分查找
    /* while (left <= right)
    {
        int mid = left + (right - left) / 2;
        if (strcmp(domain, DNSrecord[mid].domain) > 0)
            left = mid + 1;
        else if (strcmp(domain, DNSrecord[mid].domain) < 0)
            right = mid - 1;
        else
            return mid;
    } */
    for (int i = left; i < right; i++)
    {
        if (!DNSrecord[i].domain)
            continue;
        else if (!strcmp(domain, DNSrecord[i].domain))
        {
            if (DNSrecord[i].ttl < difftime(time(NULL), DNSrecord[i].recordTime))
                return i;
            else
            {
                clearRecord(i);
                return -1;
            }
        }
    }
    return -1;
}
void sendBack(char *message, int pos, int length)
{
    char sendMessage[MESSAGESIZE] = {0};
    if (DNSrecord[pos].ttl < difftime(time(NULL), DNSrecord[pos].recordTime))
    {
        sendToServer(message, length);
        return;
    }

    memcpy(sendMessage, message, length);

    unsigned short tmp;
    if (!strcmp(DNSrecord[pos].ip[0], "0.0.0.0"))
    {
        tmp = htons(0x8183);
        memcpy(&sendMessage[2], &tmp, sizeof(tmp));
        tmp = htons(0);
        memcpy(&sendMessage[6], &tmp, sizeof(tmp));
    }
    else
    {
        tmp = htons(0x8580);
        memcpy(&sendMessage[2], &tmp, sizeof(tmp));
        tmp = htons(DNSrecord[pos].sum);
        memcpy(&sendMessage[6], &tmp, sizeof(tmp));
    }
    //未修改Authority RR和Additional RR

    int i;
    for (i = 0; i < DNSrecord[pos].sum; i++)
    {
        unsigned short name = htons(0xc00c);
        //一组ip地址占用16字节
        memcpy(sendMessage + length + i * 16, &name, sizeof(name));

        unsigned short type = htons(1);
        memcpy(sendMessage + length + i * 16 + 2, &type, sizeof(type));

        unsigned short class = htons(1);
        memcpy(sendMessage + length + i * 16 + 4, &class, sizeof(class));

        unsigned int ttl = htonl((unsigned int)difftime(DNSrecord[pos].ttl, difftime(time(NULL), DNSrecord[pos].recordTime)));
        memcpy(sendMessage + length + i * 16 + 6, &ttl, sizeof(ttl));

        unsigned short dataLength = htons(4);
        memcpy(sendMessage + length + i * 16 + 10, &dataLength, sizeof(dataLength));

        unsigned int ip = (unsigned int)inet_addr(DNSrecord[pos].ip[i]);
        memcpy(sendMessage + length + i * 16 + 12, &ip, sizeof(ip));
    }

    int sendLength = sendto(sock, sendMessage, length + i * 16, 0, (SOCKADDR *)&tempAddr, sizeof(tempAddr));
    if (sendLength < 0)
        printf("#Error %d\n", WSAGetLastError());
    else if (debugLevel == 2)
    {
        printf("SEND to %s:%d(%dbytes)  ", inet_ntoa(tempAddr.sin_addr), ntohs(tempAddr.sin_port), sendLength);
        for (int i = 0; i < sendLength; i++)
            outputByBit(sendMessage[i]);

        /* printf("%x ", sendMessage[i]);*/
        printf("\n\t");
        printf("ID:");
        outputByBit(sendMessage[0]);
        outputByBit(sendMessage[1]);
        printf("QR:%d, ", 1);
        printf("OPCODE:%d, ", 0);
        printf("AA:%d, ", 1);
        printf("TC:%d, ", 0);
        printf("RD:%d, ", 1);
        printf("RA:%d, ", 1);
        printf("Z:%d, ", 0);
        printf("RCODE:%d\n\t", 0);
        printf("QDCOUNT:%d, ", 1);
        printf("ANCOUNT: %d, ", DNSrecord[pos].sum);
        printf("NSCOUNT:%d, ", 0);
        printf("ARCOUNT:%d\n", 0);
    }
}