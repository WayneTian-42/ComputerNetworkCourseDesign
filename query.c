#include "init.h"
#include "request.h"
#include "query.h"

int changeID(char *message)
{
    unsigned short id;
    memcpy(&id, message, sizeof(id));
    unsigned short tmp;
    do
    {
        srand(time(NULL));
        tmp = rand() % 1000 + 1;
    } while (userRord[tmp].originalID);
    userRord[tmp].originalID = ntohs(id);
    userRord[tmp].userAddr = tempAddr;
    return tmp;
}
void sendToServer(char *message, int length)
{
    unsigned short newID = htons(changeID(message));
    memcpy(message, &newID, sizeof(newID));
    int sendLength = sendto(sock, message, length, 0, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
    if (sendLength < 0) {}
    else
    {
    }
}
void processMessage(char *message)
{
    char rcode, type;
    short anCount, finalType;
    memcpy(&rcode, message + 3, sizeof(rcode));
    rcode = ntohs(rcode);
    memcpy(&anCount, message + 6, sizeof(anCount));
    anCount = ntohs(anCount);
    memcpy(&type, message + HEADERSIZE, sizeof(type));
    int off = type + HEADERSIZE + 1;
    while (type != 0)
    {
        memcpy(&type, message + off, sizeof(type));
        off += type + 1;
    }
    memcpy(&finalType, message + off + 1, sizeof(finalType));
    // rocde不为0表示出错，但是rcode只占4字节
    // ANCOUNT表示回答数，finalType表示类型，为1时表示TypeA即IPV4
    if (!(rcode & 0x0F) || !anCount)
        return;
    if (finalType == 1)
    {
        unsigned int pos;
        for (pos = 0; pos < recordNum; pos++)
            if (DNSrecord[pos].ttl < difftime(time(NULL), DNSrecord[pos].recordTime))
                break;
        if (pos >= 1000)
            return;
        ipv4Message(message, pos, anCount, off + 4);  //加4是type和class字段，之后就是name（指针）
    }
    else
        return;
}
//这里会出现莫名其妙的bug
void ipv4Message(char *message, int pos, int anCount, int off)
{
    short type;
    char iptmp[4] = {0}, ip[16] = {0};
    int i, count = 0;
    for (i = 0; i < anCount && count < 20; i++, off += 16)
    {
        memcpy(&type, message + off + 4, sizeof(type));
        if (type == 1)
        {
            unsigned int ttl;
            memcpy(&ttl, message + off + 6, sizeof(ttl));
            DNSrecord[pos].ttl = ntohs(ttl);

            memcpy(iptmp, message + off + 12, sizeof(iptmp));
            SOCKADDR_IN addrtmp;
            memset(&addrtmp, 0, sizeof(addrtmp));
            memcpy(&addrtmp.sin_addr.s_addr, iptmp, sizeof(iptmp));
            memcpy(ip, inet_ntoa(addrtmp.sin_addr), sizeof(ip));

            DNSrecord[pos].ip[i] = (char *)malloc((sizeof(char)) * (strlen(ip) + 1));
            memcpy(DNSrecord[pos].ip[i], ip, sizeof(ip));
            count++;
        }
    }
    if (count)
    {
        char domain[256];
        memset(domain, 0, sizeof(domain));
        getDomain(message + HEADERSIZE, domain);
        DNSrecord[pos].recordTime = time(NULL);
        int size = strlen(domain);
        DNSrecord[pos].domain = (char *)malloc(sizeof(char) * (strlen(domain) + 1));
        strcpy_s(DNSrecord[pos].domain, sizeof(domain), domain);
        if (pos > recordNum)
            recordNum++;
        DNSrecord[pos].sum = count;
    }
    else
        clearRecord(pos);
    qsort(DNSrecord, recordNum, sizeof(RECORD), cmp);
}
void recvMessage(char *message, int length)
{
    unsigned short newID;
    memcpy(&newID, message, sizeof(newID));
    unsigned short oldID = htons(userRord[ntohs(newID)].originalID);
    memcpy(message, &oldID, sizeof(oldID));
    tempAddr = userRord[ntohs(newID)].userAddr;

    int sendLength = sendto(sock, message, length, 0, (SOCKADDR *)&(tempAddr), sizeof(tempAddr));
    if (sendLength < 0) {}
    else
    {
    }
}
void clearRecord(int pos)
{
    if (DNSrecord[pos].domain)
    {
        free(DNSrecord[pos].domain);
        DNSrecord[pos].domain = NULL;
    }
    DNSrecord[pos].recordTime = 0;
    DNSrecord[pos].ttl = 0;
    for (int i = 0; i < 10; i++)
    {
        if (DNSrecord[pos].ip[i] != NULL)
        {
            free(DNSrecord[pos].ip[i]);
            DNSrecord[pos].ip[i] = NULL;
        }
    }
    DNSrecord[pos].sum = 0;
}