#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


#ifndef PROCESS_H
#define PROCESS_H

#include "log.h"

void
daemonize(void);

#endif
