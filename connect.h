#ifndef CS631_HTTP1_0_CONNECT_H
#define CS631_HTTP1_0_CONNECT_H

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
#include "sws.h"


#define DEBUG_BACKLOG 5      //TODO: how to determine backlog size?
#define SLEEP 5			//TODO: remove

int
verifyIp(const char *str);

int
open4Socket();

int
open6Socket();

void
handle4Socket(int s);

void
handle6Socket(int s);

void
selectSocket();

void
debugSocket();



#endif
