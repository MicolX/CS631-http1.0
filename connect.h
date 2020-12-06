#ifndef CONNECT_H
#define CONNECT_H

#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "connect.h"
#include "log.h"
#include "parser.h"
#include "response.h"
#include "sws.h"

#define DEBUG_BACKLOG 10
#define SLEEP 5

extern int domain;


int
verifyIp(const char *str);

int
openSocket();

//int
//open4Socket();
//
//int
//open6Socket();

void
handleSocket(int socket);

void
startServer();

//void
//handle4Socket(int s);
//
//void
//handle6Socket(int s);

//void
//selectSocket();
//
//void
//debugSocket();

#endif
