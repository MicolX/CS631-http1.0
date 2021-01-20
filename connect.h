#ifndef CONNECT_H
#define CONNECT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <err.h>
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

int
verifyIp(const char *str);

void
startServer(void);

#endif
