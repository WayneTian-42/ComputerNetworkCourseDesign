#pragma once
#include "init.h"
#include "query.h"

/**
 * @brief 在本地记录中查找
 *
 * @param domain 域名
 * @param num 本地记录数量
 * @return int 所在位置，-1表示不存在
 */
int searchLocal(char *domain, int num);
/**
 * @brief 发送回客户端
 *
 * @param message 报文
 * @param pos 本地缓存位置
 * @param length 报文长度
 */
void sendBack(char *message, int pos, int length);