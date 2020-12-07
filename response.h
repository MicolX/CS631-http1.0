#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <magic.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "parser.h"

#ifndef RESPONSE_H
#define RESPONSE_H

typedef struct response_t {
        const char *status;
        time_t lastmtime;
        const char *contenttype;
        long long contentlength;
        int headonly;
        int dirindex;
} Response;

#endif

int respond(char *, Request *, Response *);
int reply(int, Request *, Response *);
int runcgi(int, char *, char *);
int userdirhandler(char *, char[]);
