#include <WinSock2.h>
#include <minwindef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#define PORT 53
#define MESSAGESIZE 1024
#define HEADERSIZE 12
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

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

char DNSServer[] = "10.3.9.5";
char localFile[] = "D:\\VS-Code\\Vs-Code-C\\Semester_4\\computerNetwork\\DNS\\dnsrelay.txt";
int recordNum;
SOCKET sock;
SOCKADDR_IN clientAddr, serverAddr, tempAddr;
RECORD DNSrecord[1000];
USER userRord[65536];

int initSock();
int readFile();
int cmp(const void *, const void *);
int request(char *);
void getDomain(char *, char *);
int searchLocal(char *, int);
void sendBack(char *, int, int);
void sendToServer(char *, int);
int changeID(char *);
void processMessage(char *);
void ipv4Message(char *, int, int, int);
void recvMessage(char *, int);
void clearRecord(int);

int main()
{
    if (initSock() == -1)
        return -1;
    recordNum = readFile();
    char message[MESSAGESIZE];
    memset(message, 0, sizeof(message));
    char ip[32];
    memset(ip, 0, sizeof(ip));

    while (1)
    {
        /* BOOL bNewBehavior = FALSE;
        DWORD dwBytesReturned = 0;
        WSAIoctl(sock, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL); */
        int tempLength = sizeof(tempAddr);
        // temp用来记录当前信息的来源地址
        int messageLength = recvfrom(sock, message, sizeof(message), 0, (SOCKADDR *)&tempAddr, &tempLength);
        if (messageLength < 0)
        {
            printf("Error! ReceiveLength < 0!\n");
            continue;
        }

        //判断是查询报文还是响应报文

        char domain[128];
        memset(domain, 0, sizeof(domain));
        //查询报文
        if (request(message))
        {
            printf("1\n");
            //获取想要查询的域名
            getDomain(message + HEADERSIZE, domain);
            for (int i = 1; domain[i] != 0; i++)
                printf("%c", domain[i]);
            printf("\n");
            //判断本地是否缓存该域名
            int pos = searchLocal(domain + 1, recordNum);
            if ((pos != -1) && (message[messageLength - 3] == 1))
            {
                sendBack(message, pos, messageLength);
            }
            else
            {
                printf("2");
                sendToServer(message, messageLength);
            }
        }
        else  //响应报文
        {
            printf("0");
            //处理报文
            processMessage(message);
            recvMessage(message, messageLength);
            // showIP(ip);
        }
    }
    printf("fuck0");
    return 0;
}

int initSock()
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
int readFile()
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
int cmp(const void *a, const void *b)
{
    return strcmp(((RECORD *)a)->domain, ((RECORD *)b)->domain);
}
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
            domain[i + totalLength] = message[i + totalLength];
        // tmp记录next，因为马上next要更新
        int tmp = nextLength;
        nextLength = message[totalLength + nextLength + 1];
        //域名以00结尾，非0就可以在输出时用.来代替
        if (nextLength)
            domain[i + totalLength] = '.';
        else
            break;
        totalLength += tmp + 1;
    }
}
int searchLocal(char *domain, int num)
{
    int left = 0, right = num;
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

        puts("message:");
        for (int ii = 0; ii < length + i * 16; ii++)
        {
            printf("%d ", (int)sendMessage[ii]);
        }
        putchar('\n');
    }

    int sendLength = sendto(sock, sendMessage, length + i * 16, 0, (SOCKADDR *)&tempAddr, sizeof(tempAddr));
    if (sendLength < 0) {}
    else
    {
    }
}
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
    unsigned int pos;
    for (pos = 0; pos < 1000; pos++)
        if (DNSrecord[pos].ttl < difftime(time(NULL), DNSrecord[pos].recordTime))
            break;
    if (pos >= 1000)
        return;
    else
        clearRecord(pos);

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
        ipv4Message(message, pos, anCount, off + 4);  //加4是type和class字段，之后就是name（指针）
    else
        return;
}
//这里会出现莫名其妙的bug
void ipv4Message(char *message, int pos, int anCount, int off)
{
    char domain[256];
    getDomain(message, domain);
    DNSrecord[pos].recordTime = time(NULL);
    DNSrecord[pos].domain = malloc(sizeof(char) * (strlen(domain) + 1));
    strcpy(DNSrecord[pos].domain, domain);
    if (pos > recordNum)
        recordNum++;
    short type;
    char iptmp[4] = {0}, ip[16] = {0};
    int i;
    for (i = 0; i < anCount && i < 20; i++, off += 16)
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

            DNSrecord[pos].ip[i] = malloc(strlen(ip));
            memcpy(DNSrecord[pos].ip[i], ip, sizeof(ip));
        }
        if (i)
        {
            DNSrecord[pos].sum += i;
        }
    }
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
