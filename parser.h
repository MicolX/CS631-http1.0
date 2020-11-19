#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PARSER_H
#define PARSER_H

typedef struct request_t {
	char method;
	char uri[512];
	float version;
	char ifms[512];
	char *body;
	int errcode;
} Request;


int parse(char *, Request *);

#endif
