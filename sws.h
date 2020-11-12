#ifndef CS631_HTTP1_0_SWS_H
#define CS631_HTTP1_0_SWS_H

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct options_t {
        int c_opt;
        int d_opt;
        int h_opt;
        int i_opt;
        int l_opt;
        int p_opt;
} options;

extern int port;
extern char *dir, *addr, *file;




#endif
