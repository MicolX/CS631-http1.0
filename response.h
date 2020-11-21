#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "parser.h"

#ifndef RESPONSE_H
#define RESPONSE_H

typedef struct response_t {
	char *status;
	time_t lastmtime;
	char contenttype[64];
	long long contentlength;
	bool headonly;
} Response;

#endif

