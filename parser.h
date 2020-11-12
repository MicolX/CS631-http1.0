#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct request_t {
	char method[5];
	char uri[512];
	char version[9];
	char ifms[512];
	char *body;
} Request;

int parse(char *, Request *);
