/*
 * parse.c
 * Parses and interprets incoming requests made to the server.
 *
 * Mingyao Xiong
 * Liam Brew
 *
 * Based on material from Stevens Institute of Technology's
 * CS 631 APUE during the Fall 2020 semester.
 */

#include "parser.h"

#define CGI "/cgi-bin/"
#define HEADERSIZE 1
#define LIMIT 2
#define VERSION10 "HTTP/1.0"
#define VERSION09 "HTTP/0.9"

const char* Headers[HEADERSIZE] = {
        "If-Modified-Since: ",
};

int getrline(char *, const char *, Request *);
int getrheader(char *, const char *, Request *)

/*
 * Interprets the method, errcode, URI, and version data of a request.
 */
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
                req->method = UNSUPPORTED;
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
        } else if (strlen(p) == 2 && strncmp(p, "\r\n", 2)) {
				req->version = 0.9;
		} else {
                req->errcode = 400;
                return -1;
        }

        return 0;
}

/*
 * Interprets the ifms data of the request.
 */
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

/*
 * Fully parses the request and saves its data in the corresponding struct.
 */
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
