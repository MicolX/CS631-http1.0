#ifndef SWS_H
#define SWS_H

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "connect.h"
#include "log.h"
#include "sws.h"

extern int c_opt, d_opt, h_opt, i_opt, l_opt, p_opt, port, logFd, ipv, rootfd;
extern char **environ, *dir, *addr, *file, *ipAddr, *cgiDir;


int
testDir(char *dir);


#endif
