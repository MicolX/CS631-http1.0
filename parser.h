#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PARSER_H
#define PARSER_H

typedef enum Method {
	GET,
	HEAD,
	UNSUPPORT
} Method;

typedef struct request_t {
	enum Method method;
	char uri[512];
	float version;
	char ifms[512];
	int errcode;
} Request;


int parse(char *, Request *);

#endif
