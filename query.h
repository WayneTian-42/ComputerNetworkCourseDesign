#pragma once
#include "init.h"

void sendToServer(char *, int);
int changeID(char *, unsigned char *);
void processMessage(char *, HEADER *);
void ipv4Message(char *, int, int, int);
void recvMessage(char *, int);
void clearRecord(int);