#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PARSER_H
#define PARSER_H

#define MAX_PATH  255
#define MAX_TIME  30

typedef enum Method {
        GET,
        HEAD,
        UNSUPPORTED
} Method;

typedef struct request_t {
        enum Method method;
        char uri[MAX_PATH];
        float version;
        char ifms[MAX_TIME];
        int errcode;
} Request;

int parse(char *, Request *);

#endif