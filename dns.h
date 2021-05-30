#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include "init.h"
#include "request.h"
#include "query.h"

typedef _Bool bool;
typedef unsigned long long ullsize_t;
typedef struct timeb TIME;

TIME start;

/**
 * @brief 判断输入参数(IP地址)是否合法
 *
 * @param argc 参数个数
 * @param argv 输入参数
 * @return true 参数合法
 * @return false 参数不合法
 */
bool debug(int argc, char **argv);
/**
 * @brief 判断参数是否只含有数字和点
 *
 * @param str 输入参数
 * @return true 参数为ip地址
 * @return false 参数不是ip地址
 */
bool judgeForm(char *str);
/**
 * @brief 判断ip地址是否合法
 *
 * @param str 输入参数
 * @return true ip地址合法
 * @return false ip地址不合法
 */
bool isIPAddress(char *str);
/**
 * @brief dns中继程序
 *
 */
void dnsRelay();
/**
 * @brief 输出时间
 *
 */
void outTime();
