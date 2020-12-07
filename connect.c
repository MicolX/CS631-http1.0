#include "connect.h"

#define CGIPREFIX "/cgi-bin"

int domain;

int
verifyIp(const char *str) {
        if ((inet_pton(AF_INET, str, &ipAddr)) == 1) {
                return 4;
        } else if ((inet_pton(AF_INET6, str, &ipAddr)) == 1) {
                return 6;
        } else {
                return -1;
        }
}

int
openSocket() {
        int sock;
        void *s;
        socklen_t length, s_size;
        struct sockaddr_storage server;

        /* Default is IPv6, but we then become dual
         * stack by disabling IPV6_ONLY on the socket. */
        if (ipv == 4) {
                domain = PF_INET;
        } else {
                domain = PF_INET6;
        }

        if ((sock = socket(domain, SOCK_STREAM, 0)) < 0) {
                syslog(LOG_INFO, "Error creating IPv%d socket", ipv);
                exit(EXIT_FAILURE);
        }

        if (domain == PF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *) &server;
                sin->sin_family = PF_INET;
                sin->sin_addr.s_addr = INADDR_ANY;
                sin->sin_port = port;
                s = sin;
                s_size = sizeof(*sin);
        } else {
                struct sockaddr_in6 *sin = (struct sockaddr_in6 *) &server;
                sin->sin6_family = PF_INET6;
                sin->sin6_addr = in6addr_any;
                sin->sin6_port = port;
                s = sin;
                s_size = sizeof(*sin);

                /* Neither v4 nor v6 was explicitly
                 * requested, so we do both. */
                if (i_opt == 1) {
                        int off = 0;
                        if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &off, sizeof(off)) < 0) {
                                syslog(LOG_INFO, "Error setting socket option for both IPv values");
                                exit(EXIT_FAILURE);
                        }
                }
        }

        if (bind(sock, (struct sockaddr *) s, s_size) != 0) {
                syslog(LOG_INFO, "Error binding socket");
                exit(EXIT_FAILURE);
        }

        /* Find out assigned port number and print it out */
        length = sizeof(server);
        if (getsockname(sock, (struct sockaddr *) &server, &length) != 0) {
                syslog(LOG_INFO, "Error getting socket name");
                exit(EXIT_FAILURE);
        }


        if (listen(sock, DEBUG_BACKLOG) < 0) {
                syslog(LOG_INFO, "Error listening on socket");
                exit(EXIT_FAILURE);
        }

        return sock;
}


void
handleSocket(int sock) {
        int sockFd, rval;
        socklen_t size;
        socklen_t len;
        time_t timer;

        const char *rip;
        char buf[BUFSIZ];
        char claddr[INET6_ADDRSTRLEN];

        struct sockaddr_storage addr;
        struct sockaddr_in6 client;

        struct tm *gtime;
        Request *request = (Request *) malloc(sizeof(Request));
        Response *response = (Response *) malloc(sizeof(Response));

        if (request == NULL) {
                syslog(LOG_INFO, "Error allocating memory to request structure");
                exit(EXIT_FAILURE);
        }
        if (response == NULL) {
                syslog(LOG_INFO, "Error allocating memory to response structure");
                exit(EXIT_FAILURE);
        }

        size = sizeof(client);
        if ((sockFd = accept(sock, (struct sockaddr *) &client, &size)) < 0) {
                syslog(LOG_INFO, "Error accepting connection on socket");
                exit(EXIT_FAILURE);
        }

        /* No loop here as per instruction connections are terminated after requests are served. */

        bzero(buf, sizeof(buf));
        if ((rval = read(sockFd, buf, BUFSIZ)) < 0) {
                syslog(LOG_INFO, "Error reading socket");
                return;
        }

        if (rval == 0) {
                return;
        }

        len = sizeof(addr);
        if (getpeername(sockFd, (struct sockaddr *) &addr, &len) < 0) {
                syslog(LOG_INFO, "Error getting peer name");
                return;
        }

        if (domain == PF_INET) {
                struct sockaddr_in *s = (struct sockaddr_in *) &addr;
                port = ntohs(s->sin_port);
                rip = inet_ntop(PF_INET, &s->sin_addr, claddr, sizeof(claddr));
        } else {
                struct sockaddr_in6 *s = (struct sockaddr_in6 *) &addr;
                port = ntohs(s->sin6_port);
                rip = inet_ntop(PF_INET6, &s->sin6_addr, claddr, sizeof(claddr));
        }

        if (rip == NULL) {
                syslog(LOG_INFO, "inet_ntop error");
                rip = "unknown";
        }

        if (time(&timer) == -1) {
                syslog(LOG_INFO, "Error getting current time");
                return;
        }

        if ((gtime = gmtime(&timer)) == NULL) {
			syslog(LOG_INFO, "Error converting current time to GMT time");
			return;
        }

        if (parse(buf, request) == -1) {
			syslog(LOG_INFO, "Error parsing response");
        }

        if (strchr(request->uri, (int)'~') != NULL) {
			request->hastilde = 1;
			char *old = strdup(request->uri);
			if (userdirhandler(old, request->uri) == -1) {
					syslog(LOG_INFO, "Error fetching home directory");
			}
        }

        if (strncmp(request->uri, CGIPREFIX, strlen(CGIPREFIX)) == 0 && c_opt == 1) {
			char *uri = request->uri;
			(void)strsep(&uri, "n");

			if (runcgi(sockFd, uri, cgiDir) == -1) {
					syslog(LOG_INFO, "Error running cgi : %m");
			}
        } else {

			if (respond(dir, request, response) == -1) {
					syslog(LOG_INFO, "Error composing response : %m");
			}

			if (reply(sockFd, request, response) == -1) {
					syslog(LOG_INFO, "Error sending response: %m");
			}
        }

        if (time(&timer) == -1) {
                syslog(LOG_INFO, "Error getting current time: %m");
        }

        if (writeLog(rip, gtime, strtok(buf, "\n"), response->status, response->contentlength) == -1) {
                syslog(LOG_INFO, "Error converting current time to GMT time");
        }


        free(request);
        close(sockFd);
}

void
startServer() {
        int socket;

        socket = openSocket();

        for (;;) {

                if (d_opt == 1) {
                        handleSocket(socket);
                } else {
                        fd_set ready;
                        struct timeval to;

                        FD_ZERO(&ready);
                        FD_SET(socket, &ready);
                        to.tv_sec = SLEEP;
                        to.tv_usec = 0;
                        if (select(socket + 1, &ready, 0, 0, &to) < 0) {
                                syslog(LOG_INFO, "Error selecting socket: %m");
                                continue;
                        }
                        if (FD_ISSET(socket, &ready)) {
                                handleSocket(socket);
                        }
                }

        }
}
