#include "response.h"

#define MESSAGESIZE 256

const char* status[] = {
        "200 OK\r\n",						// 0
        "201 Created\r\n",					// 1
        "202 Accepted\r\n",					// 2
        "204 No Content\r\n",				// 3
        "301 Moved Permanently\r\n",		// 4
        "302 Moved Temporarily\r\n",		// 5
        "304 Not Modified\r\n",				// 6
        "400 Bad Request\r\n",				// 7
        "401 Unauthorized\r\n",				// 8
        "403 Forbidden\r\n",				// 9
        "404 Not Found\r\n",				// 10
        "500 Internal Server Error\r\n",	// 11
        "501 Not Implemented\r\n",			// 12
        "502 Bad Gateway\r\n",				// 13
        "503 Service Unavailable\r\n"		// 14
};


int
respond(char *rootpath, Request *req, Response *res) {
        if (req->errcode != 0) {
                switch (req->errcode) {
                        case 400:
                                res->status = status[7];
                                break;
                        case 501:
                                res->status = status[12];
                                break;
                }
                res->headonly = 1;
                return 0;
        }

        struct stat sb;
        struct tm ifmtime;
        int fd;
        magic_t cookie;
        char uri[MAXPATHLEN];

        snprintf(uri, MAXPATHLEN, "%s/%s", rootpath, req->uri);

        if (stat(uri, &sb) == -1) {
                switch (errno) {
                        case EACCES:
                                // log error
                                res->status = status[9];
                                break;
                        default:
                                // log error
                                res->status = status[10];
                                break;
                }
                res->headonly = 1;
                return 0;
        }

        if (S_ISDIR(sb.st_mode)) {
                char htmlpath[MAXPATHLEN];

                snprintf(htmlpath, MAXPATHLEN, "%s/index.html", uri);

                if (access(htmlpath, F_OK) != 0) {
                        res->dirindex = 1;
                        res->status = status[10];
                        strlcpy(req->uri, uri, MAXPATHLEN);
                        return 0;
                } else {
                        res->dirindex = 0;
                        strlcpy(req->uri, htmlpath, MAXPATHLEN);
                }
        } else {
                res->dirindex = 0;
                strlcpy(req->uri, uri, MAXPATHLEN);
        }

		if (stat(req->uri, &sb) == -1) {
			// log error
			return -1;
		}
        
		res->lastmtime = sb.st_mtime;

        if (strlen(req->ifms) > 0) {
                if (strptime(req->ifms, "%a, %d %b %Y %T GMT", &ifmtime) == NULL) {
                        if (strptime(req->ifms, "%a, %d-%b-%Y %T GMT", &ifmtime) == NULL) {
                                if (strptime(req->ifms, "%a %b  %d %T %Y", &ifmtime) == NULL) {
                                        res->status = status[7];
                                        res->headonly = 1;
                                        return 0;
                                }
                        }
                }

                if (difftime(sb.st_mtime, mktime(&ifmtime)) > 0) {
                        res->headonly = 0;
                } else {
                        res->status = status[6];
                        res->headonly = 1;
                        return 0;
                }
        }


        if ((fd = open(req->uri, O_RDONLY)) < 0) {
                // log error
                return -1;
        }

        res->contentlength = (long long)lseek(fd, 0, SEEK_END);

        if ((cookie = magic_open(MAGIC_MIME)) == NULL) {
                // log error
                (void)close(fd);
                return -1;
        }

        if (magic_load(cookie, NULL) != 0) {
                // log error
                (void)magic_close(cookie);
                (void)close(fd);
                return -1;
        }

        res->contenttype = magic_descriptor(cookie, fd);
        res->status = status[0];
        if (req->method == HEAD) {
                res->headonly = 1;
        }
        return 0;
}


int
(compar)(const FTSENT **a, const FTSENT **b)
{
        return strcmp((*a)->fts_name, (*b)->fts_name);
}


int
reply(int socket, Request *req, Response *res) {
        char message[MESSAGESIZE];
        char buf[BUFSIZ];
        int fd;
        ssize_t readsize;
        ssize_t writesize;

		if (req->errcode != 0) {
			snprintf(message, sizeof(message), "HTTP/1.0 %s\r\n", res->status);
			if (write(socket, message, strlen(message)) != (signed int)strlen(message)) {
				// log error
				return -1;
			}
		}

        if (req->version == 1.0) {
                if (res->dirindex == 0 && strcmp(res->status, "200 OK\r\n") == 0) {
                        char curtime[MAX_TIME];
                        char mtime[MAX_TIME];
                        struct tm *curtm;
                        struct tm *mtm;
                        time_t now;

                        now = time(0);
                        curtm = localtime(&now);
                        strftime(curtime, sizeof(curtime), "%a, %d %b %Y %T GMT", curtm);

                        mtm = localtime(&(res->lastmtime));
                        strftime(mtime, sizeof(curtime), "%a, %d %b %Y %T GMT", mtm);

                        snprintf(message, sizeof(message), "HTTP/1.0 %s\r\nDate: %s\r\nServer: SAS(Super Awesome Server)\r\nLast-Modified: %s\r\nContent-Type: %s\r\nContent-Length: %lld\r\n\r\n", res->status, curtime, mtime, res->contenttype, res->contentlength);

                } else {
                        snprintf(message, sizeof(message), "HTTP/1.0 %s\r\n", res->status);
                }

                if (write(socket, message, strlen(message)) != (signed int)strlen(message)) {
                        // log error
                        return -1;
                }
        }


        if (res->dirindex == 1) {
                FTSENT *ent;
                FTS *ftsp;
                char *dir[2];
                dir[0] = req->uri;
                dir[1] = NULL;

                ftsp = fts_open(dir, FTS_PHYSICAL, compar);
                errno = 0;
                while ((ent = fts_read(ftsp))) {
                        if (errno) {
                                // log error
                                (void)fts_close(ftsp);
                                return -1;
                        }
                        if (ent->fts_level == 1 && ent->fts_info != FTS_DP) {
                                char fname[strlen(ent->fts_name)+2];
                                (void)snprintf(fname, sizeof(fname), "%s\n", ent->fts_name);

                                if (write(socket, fname, strlen(fname)) != (int) strlen(fname)) {
                                        // log error
                                        (void)fts_close(ftsp);
                                        return -1;
                                }
                        }
                }
                (void)fts_close(ftsp);
                return 0;
        }

        if (req->method == GET) {
                if ((fd = open(req->uri, O_RDONLY)) == -1) {
                        // log error
                        return -1;
                }

                while ((readsize = read(fd, buf, sizeof(buf))) > 0) {
                        if ((writesize = write(socket, buf, readsize)) != readsize) {
                                // log error
                                (void)close(fd);
                                return -1;
                        }
                }

                (void)close(fd);
        }

        return 0;
}

int
runcgi(int socket, char *uri, char *dir) {
        char *path, fullpath[MAXPATHLEN];
        pid_t pid;

        path = strsep(&uri, "?");
        snprintf(fullpath, MAXPATHLEN, "%s/%s", dir, path);

        if ((pid = fork()) < 0) {
                // log error
                return -1;
        } else if (pid == 0) {

                if (uri != NULL && strlen(uri) > 0) {
                        char *var;
                        while ((var = strsep(&uri, "&")) != NULL) {
                                if (putenv(var) == -1) {
                                        // log error
                                        return -1;
                                }
                        }
                }

                if (dup2(socket, STDOUT_FILENO) < 0) {
                        // log error
                        exit(EXIT_FAILURE);
                }

                if (dup2(socket, STDERR_FILENO) < 0) {
                        // log error
                        exit(EXIT_FAILURE);
                }

                char command[MAXPATHLEN];
                snprintf(command, MAXPATHLEN, "%s", fullpath);
                execl("/bin/sh", "sh", "-c", command, (char*) 0);
                return -1;
        }

        (void)wait(NULL);
        return 0;
}

int
userdirhandler(char *uri, char newuri[]) {
        char *before, *uname;
        struct passwd *psw;

        before = strsep(&uri, "~");
        uname = strsep(&uri, "/");

        if ((psw = getpwnam(uname)) == NULL) {
                // log error
                return -1;
        }

        if (before != NULL && before[0] != '\0') {
                if (uri != NULL) {
                        snprintf(newuri, MAXPATHLEN, "%s/%s/sws/%s", before, psw->pw_dir, uri);
                } else {
                        snprintf(newuri, MAXPATHLEN, "%s/%s/sws", before, psw->pw_dir);
                }
        } else {
                if (uri != NULL) {
                        snprintf(newuri, MAXPATHLEN, "%s/sws/%s", psw->pw_dir, uri);
                } else {
                        snprintf(newuri, MAXPATHLEN, "%s/sws", psw->pw_dir);
                }
        }

        return 0;
}


//int
//main(int argc, char **argv) {
//        Response *res = (Response *)malloc(sizeof(Response));
//        Request *req = (Request *)malloc(sizeof(Request));
//        req->errcode = 0;
//        char *rootdir = "/home/mingyao/sws";
//        char *cgidir = "/home/mingyao/mid-term";
//        char *cgiuri = strdup("/mxiong3/ls");
//        int fd;
//
//        if (parse("GET /main HTTP/1.0\r\nIf-Modified-Since: Fri, 04 Dec 2010 18:40:37 GMT\r\n", req) == -1) {
//                printf("parse failed\n");
//                return 0;
//        }
//
//        printf("========== request info ===========\n");
//        if (req->method == GET) {
//                printf("method = GET\n");
//        } else if (req->method == HEAD) {
//                printf("method = HEAD\n");
//        } else {
//                printf("method = UNSUPPORTED\n");
//        }
//        printf("uri = %s\n", req->uri);
//        printf("version = %f\n", req->version);
//        printf("if-modified-since = %s\n", req->ifms);
//        printf("errcode = %d\n", req->errcode);
//        printf("============= end of request info ============\n");
//
////	if (chroot(rootdir) < 0) {
////		printf("chroot failed, error: %s\n", strerror(errno));
////		exit(EXIT_FAILURE);
////	}
//
////	if ((rootfd = open(rootdir, O_RDONLY|O_DIRECTORY)) == -1) {
////		printf("open root dir failed, error: %s\n", strerror(errno));
////		exit(EXIT_FAILURE);
////	}
//
//        if (respond(rootdir, req, res) == -1) {
//                printf("compose respond failed\n");
//                exit(EXIT_FAILURE);
//        }
//
//        printf("================ response info ==============\n");
//        printf("status = %s\n", res->status);
//        printf("last mtime = %s\n", ctime(&(res->lastmtime)));
//        printf("content type = %s\n", res->contenttype);
//        printf("content length = %lld\n", res->contentlength);
//        printf("head only = %d\n", res->headonly);
//        printf("dir index = %d\n", res->dirindex);
//        printf("================ end of response info ==============\n");
//
//        fd = open("socket", O_WRONLY|O_CREAT|O_TRUNC, 0777);
//
//	if (reply(fd, req, res) == 0) {
//		printf("reply successfully!!\n");
//	} else {
//		printf("reply failed\n");
//	}
//
////        if (runcgi(fd, cgiuri, cgidir) == 0) {
////                printf("cgi run succeed\n");
////        } else {
////                printf("cgi failed\n");
////        }
//
//        char *uri = strdup("~mingyao/main/index.html");
//        char new[MAXPATHLEN];
//        if (userdirhandler(uri, new) == -1) {
//                printf("handle failed\n");
//        } else {
//                printf("%s\n", new);
//        }
//
//        exit(EXIT_SUCCESS);
//}
