#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "init.h"

typedef _Bool bool;
typedef unsigned long long ullsize_t;

ullsize_t start;

bool debug(int, char **);
bool isIPAddress(char *);

void outTime();
