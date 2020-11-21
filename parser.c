#include "parser.h"

#define LIMIT 2
#define HEADERSIZE 1
#define VERSION10 "HTTP/1.0"
#define VERSION09 "HTTP/0.9"

const char* Headers[HEADERSIZE] = {
	"If-Modified-Since: ",
};

int
getrline(char *p, const char *end, Request *req)
{
	char *q;
	if (strncmp(p, "GET ", 4) == 0) {
		req->method = GET;
		p += 4;
	} else if (strncmp(p, "HEAD ", 5) == 0) {
		req->method = HEAD;
		p += 5;
	} else {
		req->method = UNSUPPORT;
		req->errcode = 501;
		return -1;
	}

	if (p >= end) {
		req->errcode = 400;
		return -1;
	}

	// when the version is 0.9
	if ((q = strchr(p, (int)' ')) == NULL) {
		(void)strlcpy(req->uri, p, end - p + 1);
		req->version = 0.9;
		return 0;
	}
	
	(void)strlcpy(req->uri, p, q - p + 1);
	
	p = q + 1;
	if (p + 8 != end) {
		req->errcode = 400;
		return -1;
	}
	if (strncmp(p, VERSION10, 8) == 0) {
		req->version = 1.0;
	} else if (strncmp(p, VERSION09, 8) == 0) {
		req->version = 0.9;
	} else {
		req->errcode = 400;
		return -1;
	}

	return 0;
}

int
getrheader(char *p, const char *end, Request *req)
{
	if (p == end) {
		req->errcode = 400;
		return -1;
	}

	char *q;
	if ((q = strchr(p, (int)' ')) == NULL) {
		return -1;
	}

	for (int i = 0; i < HEADERSIZE; i++) {
		if (strncmp(Headers[i], p, q - p) == 0) {
			if (strncmp(p, "If-Modified-Since: ", sizeof("If-Modified-Since:")) == 0) {
				q++;
				(void)strlcpy(req->ifms, q, end - q + 1);
				return 0;
			}
		}
	}
	
	req->errcode = 400;
	return -1;
}

int
parse(char *request, Request *req)
{
	const size_t len = strlen(request);
	char *start, *end;
	int line = 0;
	start = request;

	while ((end = strchr(start, (int)'\r')) != NULL) {
		if (line > LIMIT) {
			break;
		}
		
		if (((unsigned int)(end - request) >= len - 1) || (end[1] != '\n')) {
			req->errcode = 400;
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
		req->errcode = 400;
		return -1;
	}

	if ((unsigned int)(start - request) != len - 1 || start[0] != '\n') {
		req->errcode = 400;
		return -1;
	}

	return 0;
}

//int
//main(int argc, char **argv)
//{
//	Request *req = (Request *)malloc(sizeof(Request));
//	if (req == NULL) {
//		fprintf(stderr, "malloc returns null\n");
//		exit(EXIT_FAILURE);
//	}
//
//	if (parse("GET ab\r\n", req) == -1) {
//		printf("parse fail\n");
//	} else {
//		if (req->method == GET) {
//			printf("method = GET\n");
//		} else if (req->method == HEAD) {
//			printf("method = HEAD\n");
//		} else {
//			printf("method = Unsupported\n");
//		}
//		printf("uri = %s\n", req->uri);
//		printf("version = %f\n", req->version);
//		printf("if-modified-since = %s\n", req->ifms);
//	}
//}
