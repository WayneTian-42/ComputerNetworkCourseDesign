
// #include <minwindef.h>
#include "dns.h"
#include "init.h"
#include "request.h"
#include "query.h"

char DNSServer[] = "192.168.43.153";
char localFile[] = "D:\\VS-Code\\Vs-Code-C\\Semester_4\\computerNetwork\\DNS\\dnsrelay.txt";

int main(int argc, char **argv)
{
    debug(argc, argv);

    if (initSock(DNSServer) == -1)
        return -1;
    recordNum = readFile(localFile);
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
            for (int i = 0; domain[i] != 0; i++)
                printf("%c", domain[i]);
            printf("\n");
            //判断本地是否缓存该域名
            int pos = searchLocal(domain, recordNum);
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

void debug(int argc, char **argv)
{
}