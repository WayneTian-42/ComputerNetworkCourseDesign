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

bool debug(int, char **);
bool isIPAddress(char *);
void dnsRelay();

void outTime();
