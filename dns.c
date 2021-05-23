
// #include <minwindef.h>
#include <string.h>
#include <process.h>
#include <sys/timeb.h>
#include "dns.h"

char DNSServer[] = "202.106.0.20";
char localFile[] = "D:\\VS-Code\\Vs-Code-C\\Semester_4\\computerNetwork\\DNS\\dnsrelay.txt";
int num = 1;

HANDLE hMutex;

int main(int argc, char **argv)
{
    printf("DNSRELAY, Version 1.0, Build: May 21 2021 18:06:57\n");
    printf("Usage: dnsrelay [-d | -dd] [<dns-server>] [<db-file>]\n");
    if (!debug(argc, argv))
    {
        printf("The arguments are illegal!\n");
        return -1;
    }
    printf("DNS server: %s\n", DNSServer);
    printf("Debug level %d\n", debugLevel);

    if (initSock(DNSServer) == -1)
        return -1;
    recordNum = readFile(localFile);

    ftime(&start);

    _beginthread(dnsRelay, 0, NULL);
    _beginthread(dnsRelay, 0, NULL);
    _beginthread(dnsRelay, 0, NULL);
    _beginthread(dnsRelay, 0, NULL);
    dnsRelay();

    return 0;
}

bool debug(int argc, char **argv)
{
    bool flg = TRUE;
    if (argc == 1)
        return TRUE;
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-d"))
            debugLevel = 1;
        else if (!strcmp(argv[i], "-dd"))
            debugLevel = 2;
        else if (argv[i][0] >= '0' || argv[i][0] <= '9')
        {
            if (isIPAddress(argv[i]))
                memcpy(DNSServer, argv[i], sizeof(*argv[i]));
            else
            {
                flg = FALSE;
                printf("Illegal IPV4 address!\n");
            }
        }
        else if (flg)
            memcpy(DNSServer, argv[i], sizeof(*argv[i]));
    }
    return flg;
}
bool isIPAddress(char *str)
{
    return (INADDR_NONE != inet_addr(str));
}
void dnsRelay()
{
    unsigned char message[MESSAGESIZE];
    memset(message, 0, sizeof(message));
    char ip[32];
    memset(ip, 0, sizeof(ip));

    HEADER header;
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
            printf("#Error %d\n", WSAGetLastError());
            continue;
        }
        memset(&header, 0, sizeof(header));
        getHeader(&header, message);
        if (debugLevel > 1)
        {
            printf("RECV from %s:%d(%dbytes)  ", inet_ntoa(tempAddr.sin_addr), ntohs(tempAddr.sin_port), messageLength);
            for (int i = 0; i < messageLength; i++)
                outputByBit(message[i]);
            printf("\n\t");
            printf("ID:");
            printf("%x%x", (message[0] & 0xf0) >> 4, message[0] & 0x0f);
            printf("%x%x, ", (message[1] & 0xf0) >> 4, message[1] & 0x0f);
            printf("QR:%d, ", header.QR);
            printf("OPCODE:%d, ", header.OPCODE);
            printf("AA:%d, ", header.AA);
            printf("TC:%d, ", header.TC);
            printf("RD:%d, ", header.RD);
            printf("RA:%d, ", header.RA);
            printf("Z:%d, ", header.Z);
            printf("RCODE:%d\n\t", header.RCODE);
            printf("QDCOUNT:%d, ", header.QDCOUNT);
            printf("ANCOUNT: %d, ", header.ANCOUNT);
            printf("NSCOUNT:%d, ", header.NSCOUNT);
            printf("ARCOUNT:%d\n", header.ARCOUNT);
        }

        char domain[128];
        memset(domain, 0, sizeof(domain));
        //获取想要查询的域名
        getDomain(message + HEADERSIZE, domain);

        //判断是查询报文还是响应报文
        if (!header.QR)
        {
            //判断本地是否缓存该域名
            int pos = searchLocal(domain, recordNum);
            if (debugLevel > -1)
            {
                // WaitForSingleObject(hMutex, INFINITE);
                outTime();
                printf("%4d:", num++);
                if (pos == -1)
                    printf("  ");
                else
                    printf("* ");
                for (int i = 0; domain[i] != 0; i++)
                    printf("%c", domain[i]);
                printf("\n");
                // ReleaseMutex(hMutex);
            }
            if ((pos != -1) && (message[messageLength - 3] == 1))
                sendBack(message, pos, messageLength);
            else
                sendToServer(message, messageLength);
        }
        else  //响应报文
        {
            processMessage(message);
            recvMessage(message, messageLength);
        }
    }
}
void outTime()
{
    TIME end;
    ftime(&end);
    double diff = (double)(1.0 * (end.time - start.time) + (end.millitm - start.millitm) / 1000.0);
    printf("%10.3lf", diff);
}