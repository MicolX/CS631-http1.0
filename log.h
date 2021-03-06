#ifndef CS631_HTTP1_0_LOG_H
#define CS631_HTTP1_0_LOG_H

#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "connect.h"
#include "sws.h"


typedef struct log_t {
        const char *remoteIp;
        struct tm *time;
        char *firstLine;
        const char *status;
        int contentLength;
} Log;

int
writeLog(const char *rip, char time[], char *firstLine, const char *status, long long contentLength);

#endif
