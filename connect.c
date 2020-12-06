#include "connect.h"

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
        int sock, num;
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
                syslog(0, "Error creating IPv%d socket: %m", ipv);
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
                                syslog(0, "Error setting socket option for both IPv values: %m");
                                exit(EXIT_FAILURE);
                        }
                }
        }

        if (bind(sock, (struct sockaddr *) s, s_size) != 0) {
                syslog(0, "Error binding socket: %m");
                exit(EXIT_FAILURE);
        }

        /* Find out assigned port number and print it out */
        length = sizeof(server);
        if (getsockname(sock, (struct sockaddr *) &server, &length) != 0) {
                syslog(0, "Error getting socket name: %m");
                exit(EXIT_FAILURE);
        }

        if (domain == PF_INET) {
                struct sockaddr_in *s = (struct sockaddr_in *) &server;
                num = ntohs(s->sin_port);
        } else {
                struct sockaddr_in6 *s = (struct sockaddr_in6 *) &server;
                num = ntohs(s->sin6_port);
        }
        printf("Socket started on Port #%d", num);

        if (listen(sock, DEBUG_BACKLOG) < 0) {
                syslog(0, "Error listening on socket: %m");
                exit(EXIT_FAILURE);
        }

        return sock;
}


void
handleSocket(int sock)
{
        int sockFd, rval;
        struct sockaddr_in6 client;
        socklen_t size;
        Request *request = (Request *) malloc(sizeof(Request));
        Response *response = (Response *) malloc(sizeof(Response));
        Log *log = (Log *) malloc(sizeof(Log));

        if (request == NULL) {
                syslog(NULL, "Error allocating memory to request structure: %m");
                exit(EXIT_FAILURE);
        }
        if (response == NULL) {
                syslog(NULL, "Error allocating memory to response structure: %m");
                exit(EXIT_FAILURE);
        }
        if (log == NULL) {
                syslog(NULL, "Error allocating memory to log structure : %m");
                exit(EXIT_FAILURE);
        }

        size = sizeof(client);
        if ((sockFd = accept(sock, (struct sockaddr *) &client, &size)) < 0) {
                syslog(0, "Error accepting connection on socket: %m");
                exit(EXIT_FAILURE);
        }

        /* No loop here as per instruction connections are terminated after requests are served. */

        char buf[BUFSIZ];
        char claddr[INET6_ADDRSTRLEN];
        struct sockaddr_storage addr;
        socklen_t len;
        const char *rip;
        int port;
        time_t timer;
        struct tm *gmtime;

        bzero(buf, sizeof(buf));
        if ((rval = read(sockFd, buf, BUFSIZ)) < 0) {
                syslog(0, "Error reading socket: %m");
                break;
        }

        if (rval == 0) {
                break;
        }

        len = sizeof(addr);
        if (getpeername(sockFd, (struct sockaddr *) &addr, &len) < 0) {
                syslog(0, "Error getting peer name: %m");
                break;
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
                syslog(0, "inet_ntop error: %m");
                rip = "unknown";
        }

        if (parse(buf, request) == -1) {
                if (write(sockFd, INVALID_REQUEST, sizeof(INVALID_REQUEST)) < 0) {
                        syslog(0, "Error sending invalid request error message: %m");
                        break;
                }
        } else {
                if (respond(buf, request, response) == -1) {
                        syslog(0, "Error composing response : %m");
                        break;
                }

                if (reply(sockFd, request, response) == -1) {
                        syslog(0, "Error sending response: %m");
                        break;
                }

                if (time(&timer) == -1) {
                        syslog(0, "Error getting current time: %m");
                        break;
                }

                if ((gmtime = gmtime(&timer)) == NULL) {
                        syslog(0, "Error converting current time to GMT time: %m");
                        break;
                }

                log->remoteIp = rip;
                log->time = gmtime;
                log->firstLine = strtok(buf, "\n");
                /* According to manual above has no errors; NULL case is invalid and won't make it this far */
                log->status = response->status;
                log->contentLength = response->contentlength;

                if (writeLog(log) == -1) {
                        syslog(0, "Error converting current time to GMT time: %m");
                        exit(EXIT_FAILURE);
                }

        }

        free(request);
        free(log);
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
                                syslog(0, "Error selecting socket: %m");
                                continue;
                        }
                        if (FD_ISSET(socket, &ready)) {
                                handleSocket(socket);
                        }
                }

        }
}