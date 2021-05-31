#pragma once
#include "init.h"

/**
 * @brief 随机数哈希来转换ID
 *
 * @param message 报文
 * @param oldID 旧ID
 * @return int 新ID
 */
int changeID(char *message, unsigned char *oldID);
/**
 * @brief 发送到服务器
 *
 * @param message 报文
 * @param length 长度
 */
void sendToServer(char *message, int length);
/**
 * @brief 处理响应报文
 *
 * @param message 报文
 * @param header 头部
 */
void processMessage(char *message, HEADER *header);
/**
 * @brief 处理IPv4消息
 *
 * @param message 报文
 * @param pos 在DNSRecord中位置
 * @param anCount 回答数（域名对应IPv4地址数）
 * @param off ip地址在报文中偏移量
 */
void ipv4Message(char *message, int pos, int anCount, int off);
/**
 * @brief 将处理后报文发送回客户端
 *
 * @param message 报文
 * @param length 报文长度
 */
void snedAnswer(char *message, int length);
/**
 * @brief 清理ttl已超时的记录
 *
 * @param pos 在记录表中位置
 */
void clearRecord(int pos);