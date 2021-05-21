#include "init.h"
#include "request.h"
#include "query.h"

int request(char *message)
{
    unsigned short flg;
    memcpy(&flg, message + 2, sizeof(flg));
    return !((ntohs(flg)) & 0x8000);
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
            domain[i + totalLength - 1] = message[i + totalLength];
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
int searchLocal(char *domain, int num)
{
    int left = 0, right = num - 1;
    //二分查找
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        if (strcmp(domain, DNSrecord[mid].domain) > 0)
            left = mid + 1;
        else if (strcmp(domain, DNSrecord[mid].domain) < 0)
            right = mid - 1;
        else
            return mid;
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
        tmp = htons(0x8180);
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
    if (sendLength < 0) {}
    else
    {
    }
}