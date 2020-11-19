#ifndef CS631_HTTP1_0_LOG_H
#define CS631_HTTP1_0_LOG_H

#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>

#include "connect.h"
#include "sws.h"

void
writeLog(char *str);

#endif
