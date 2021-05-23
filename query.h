#pragma once

void sendToServer(char *, int);
int changeID(char *, char *);
void processMessage(char *);
void ipv4Message(char *, int, int, int);
void recvMessage(char *, int);
void clearRecord(int);