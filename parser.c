#include "parser.h"

#define LIMIT 2
#define SIZE 1
#define VERSION "HTTP/1.0"

const char* Headers[SIZE] = {
	"If-Modified-Since: ",
};

int
getrline(char *p, const char *end, Request *req) {
	char *q;
	if (strncmp(p, "GET ", 4) == 0) {
		(void)strlcpy(req->method, p, 4);
		p += 4;
	} else if (strncmp(p, "HEAD ", 5) == 0) {
		(void)strlcpy(req->method, p, 5);
		p += 5;
	} else {
		// should set error message here
		return -1;
	}

	if (p >= end) {
		// should set error message here
		return -1;
	}

	if ((q = strchr(p, (int)' ')) == NULL) {
		if ((q = strchr(p, (int)'\r')) == NULL || q != end) {
			// should set error message here
			return -1;
		}
	}
	
	(void)strlcpy(req->uri, p, q - p + 1);
	
	if (q[0] == ' ') {
		p = q + 1;
		if (p + 8 != end) {
			// should set error message here
			return -1;
		}
		if (strncmp(p, VERSION, 8) == 0) {
			(void)strlcpy(req->version, VERSION, 9);
		} else {
			// should set error message here
			return -1;
		}
	}

	return 0;
}

int
getrheader(char *p, const char *end, Request *req) {
	char *q;
	if ((q = strchr(p, (int)' ')) == NULL) {
		// should set error message here
		return -1;
	}

	for (int i = 0; i < SIZE; i++) {
		if (strncmp(Headers[i], p, q - p) == 0) {
			if (strncmp(p, "If-Modified-Since: ", sizeof("If-Modified-Since:")) == 0) {
				q++;
				(void)strlcpy(req->ifms, q, end - q + 1);
				return 0;
			}
		}
	}
	
	// should set error message here
	return -1;
}

int
parse(char *request, Request *req) {
	const size_t len = strlen(request);
	char *start, *end;
	int line = 0;
	start = request;

	while ((end = strchr(start, (int)'\r')) != NULL) {
		if (line > LIMIT) {
			break;
		}
		
		if ((end - request >= len - 1) || (end[1] != '\n')) {
			// should set error message here
			return -1;
		}
		
		if (line == 0) {
			if (getrline(start, end, req) == -1) {
				return -1;
			}
		} else {
			if (getrheader(start + 1, end, req) == -1) {
				break;
			} 
		}
		
		line++;
		start = end + 1;
	}

	if (line == 0) {
		// should set error message here
		return -1;
	}

	if (start - request != len - 1 || start[0] != '\n') {
		// should set error message here
		return -1;
	}

	return 0;
}

int
main(int argc, char **argv) {
	Request *req = (Request *)malloc(sizeof(Request));
	if (req == NULL) {
		fprintf(stderr, "malloc returns null\n");
		exit(EXIT_FAILURE);
	}

	if (parse("HEAD abc HTTP/1.0\r\nIf-Modified-Since: abc\r\n", req) == -1) {
		printf("parse fail\n");
	} else {
		printf("method = %s\n", req->method);
		printf("uri = %s\n", req->uri);
		printf("version = %s\n", req->version);
		printf("if-modified-since = %s\n", req->ifms);
	}
}
