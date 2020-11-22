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
#include "process.h"
#include "log.h"
#include "sws.h"


#ifndef CS631_HTTP1_0_SWS_H
#define CS631_HTTP1_0_SWS_H


extern int c_opt, d_opt, h_opt, i_opt, l_opt, p_opt, port, logFd, ipv;
extern char **environ, *dir, *addr, *file, *ipAddr;




#endif
