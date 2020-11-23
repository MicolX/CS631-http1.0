#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <magic.h>
#include <stdlib.h>
#include <string.h>
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
} Response;

#endif

int respond(int, Request *, Response *);
int reply(int, int, Request *, Response *);
