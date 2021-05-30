#pragma once
#include <WinSock2.h>
#include <string.h>
// #include <winsock2.h>
#include <time.h>

#define PORT 53
#define MESSAGESIZE 1024
#define HEADERSIZE 12

// 查询记录
typedef struct
{
    char *domain;
    char *ip[20];
    int sum, type;
    time_t recordTime, ttl;
} RECORD;

// 哈希表记录用户地址与IP
typedef struct
{
    SOCKADDR_IN userAddr;
    unsigned short originalID;
} USER;

// 报文头部
typedef struct
{
    unsigned ID : 16;
    unsigned QR : 1;
    unsigned OPCODE : 4;
    unsigned AA : 1;
    unsigned TC : 1;
    unsigned RD : 1;
    unsigned RA : 1;
    unsigned Z : 3;
    unsigned RCODE : 4;
    unsigned QDCOUNT : 16;
    unsigned ANCOUNT : 16;
    unsigned NSCOUNT : 16;
    unsigned ARCOUNT : 16;

} HEADER;

// 记录数量 和 调试等级
extern int recordNum, debugLevel;
SOCKET sock;                                   // socket
SOCKADDR_IN clientAddr, serverAddr, tempAddr;  // 本地地址，服务器地址，临时地址
RECORD DNSrecord[1000];                        // DNS记录
USER userRord[65536];                          // 用户ID转换记录

// char messageStructure[8];
/**
 * @brief 初始化socket
 *
 * @param DNSServer 服务器地址
 * @return int -1表示建立失败，1表示建立成功
 */
int initSock(char *DNSServer);
/**
 * @brief 读取文件内容
 *
 * @param localFile 文件地址
 * @return int 返回文件内记录个数
 */
int readFile(char *localFile);
/**
 * @brief 一字节内容按两个十六进制输出
 *
 * @param c 一字节内容
 */
void outputByBit(char c);
/**
 * @brief Get the Header object
 *
 * @param header 头部结构体
 * @param message 报文信息
 */
void getHeader(HEADER *header, char *message);
/**
 * @brief Get the Domain object
 *
 * @param message 报文信息
 * @param domain 存储域名
 */
void getDomain(char *message, char *domain);
// int cmp(const void *, const void *);