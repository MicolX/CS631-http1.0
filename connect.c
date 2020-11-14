#include "connect.h"


int
openSocket()
{
        int socket;
        socklen_t socklen;
        struct sockaddr_in6 sockaddr;

        // TODO: confirm that PF_INET6 communicates with both IPv4 and IPv6 (handles both)
        if ((socket = socket(PF_INET6, SOCK_STREAM, 0)) < 0) {
                perror("error getting socket stream");
                exit(EXIT_FAILURE);
        }

        sockaddr.sin6_family = PF_INET6;
        sockaddr.sin6_addr = in6addr_any;       //TODO: validate IP from option
        sockaddr.sin6_port = port;              //TODO: verify port from option

        if (bind(socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 0) {
                perror("error binding the socket stream");
                exit(EXIT_FAILURE);
        }

        socklen = sizeof(sockaddr);
        if (getsockname(sock, (struct sockaddr *)&server, &socklen) != 0) {
                perror("error getting socket name");
                exit(EXIT_FAILURE);
        }

        (void)printf("socket has port #%d\n", ntohs(server.sin6_port));

        if (listen)sock, DEBUG_BACKLOG) < 0) {
                perror("listening");
                exit(EXIT_FAILURE);
        }
        return socket
}

void
handleSocket(int s)
{
        int socketFd, reader;
        char addr[INET6_ADDRSTRLEN];
        struct sockaddr_in6 client;
        socklen_t socklen;
        const char *rip;

        socklen = sizeof(client);
        if ((socketFd = accept(s, (struct sockaddr *)&client, &length)) < 0) {
                perror("accept");
                return;
        }

        if ((rip = inet_ntop(PF_INET6, &(client.sin6_addr), addr, INET6_ADDRSTRLEN) == NULL) {
                perror("inet_ntop");
                rip = "unknown";
        } else {
                printf("client connected from %s\n", rip);
        }

        do {
                char buf[BUFSIZ];
                bzero(buf, sizeof(buf));
                if ((reader = read(socketFd, buf, BUFSIZ)) < 0) {
                        perror("error reading stream message");
                } else if (reader == 0) {
                        printf("ending connection from %s\n", rip);
                } else {
                        printf("Client %s sent: %s", rip, buf);
                } while (reader != 0);
                close(socketFd);
        }
}



int
debugSocket()
{
        int s1;

        s1 = openSocket();

        for (;;) {
                fd_set ready;
                struct timeval to;

                FD_ZERO(&ready);
                FD_SET(s1, &ready);
                to.tv_sec = SLEEP;
                to.tv_usec = 0;
                if (select(s1 + 1, &ready, 0, 0, &to) < 0) {
                        perror("select");
                        continue;
                }
                if (FD_ISSET(s1, &ready)) {
                        handleSocket(s1);
                } else {
                        (void)printf("Idly sitting here, waiting for connections...\n");
                }
        }
}