#ifndef CS631_HTTP1_0_SWS_H
#define CS631_HTTP1_0_SWS_H

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>     // for logging
#include <unistd.h>

extern int c_opt, d_opt, h_opt, i_opt, l_opt, p_opt, port, logFd;
extern char **environ, *dir, *addr, *file;




#endif
