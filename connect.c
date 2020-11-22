#include "connect.h"

int
verifyIp(const char *str)
{
	if ((inet_pton(AF_INET, str, &ipAddr)) == 1) {
		return 4;
	} else if ((inet_pton(AF_INET6, str, &ipAddr)) == 1) {
		return 6;
	} else {
		return -1;
	}
}

int
open4Socket() {
        int sock, type;
        socklen_t socklen;

        struct sockaddr_in sockaddr;
        struct in_addr addr;
        type = PF_INET;
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = port;

        if (i_opt == 1) {
                if (inet_aton(ipAddr, &addr) == 0) {
                        perror("setting IPv4 socket address");
                        exit(EXIT_FAILURE);
                }
                sockaddr.sin_addr = addr;
        } else {
		if (inet_aton("::fff:0.0.0.0", &addr)) {
			perror("setting catch-all IPv4 socket address");
			exit(EXIT_FAILURE);
		}
		sockaddr.sin_addr = addr;
        }

        if ((sock = socket(type, SOCK_STREAM, 0)) < 0) {
                perror("error getting socket stream");
                exit(EXIT_FAILURE);
        }

        if (bind(sock, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) != 0) {
                perror("error binding the socket stream");
                exit(EXIT_FAILURE);
        }

        socklen = sizeof(sockaddr);
        if (getsockname(sock, (struct sockaddr *) &sockaddr, &socklen) != 0) {
                perror("error getting socket name");
                exit(EXIT_FAILURE);
        }

        printf("socket has port #%d\n", ntohs(port));

        if (listen(sock, DEBUG_BACKLOG) < 0) {
                perror("listening");
                exit(EXIT_FAILURE);
        }
        return sock;
}


int
open6Socket() {
        int sock, type;
        socklen_t socklen;

        struct sockaddr_in6 sockaddr;
        struct in6_addr addr;
	
	type = PF_INET6;
        sockaddr.sin6_family = AF_INET6;
        sockaddr.sin6_port = port;

        if (i_opt == 1) {
		
                if (inet_pton(AF_INET6, ipAddr, &addr) == 0) {
                        perror("setting IPv6 socket address");
                        exit(EXIT_FAILURE);
                }
		sockaddr.sin6_addr = addr;
        } else {
                sockaddr.sin6_addr = in6addr_any;
        }


        if ((sock = socket(type, SOCK_STREAM, 0)) < 0) {
                perror("error getting socket stream");
                exit(EXIT_FAILURE);
        }

        if (bind(sock, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) != 0) {
                perror("error binding the socket stream");
                exit(EXIT_FAILURE);
        }

        socklen = sizeof(sockaddr);
        if (getsockname(sock, (struct sockaddr *) &sockaddr, &socklen) != 0) {
                perror("error getting socket name");
                exit(EXIT_FAILURE);
        }

        printf("socket has port #%d\n", ntohs(port));

        if (listen(sock, DEBUG_BACKLOG) < 0) {
                perror("listening");
                exit(EXIT_FAILURE);
        }
        return sock;
}

void
handle4Socket(int s) {
        int socketFd, reader;
        socklen_t socklen;
        const char *connectionIP;

        char addr[INET_ADDRSTRLEN];
        struct sockaddr_in client;

        socklen = sizeof(client);
        if ((socketFd = accept(s, (struct sockaddr *) &client, &socklen)) < 0) {
                perror("accept");
                return;
        }

        if ((connectionIP = inet_ntop(PF_INET, &(client.sin_addr), addr, INET_ADDRSTRLEN)) == NULL) {
                perror("inet_ntop");
                connectionIP = "unknown";
        } else {
                printf("client connected from %s\n", connectionIP);
        }

        do {
                char buf[BUFSIZ];
                bzero(buf, sizeof(buf));
                if ((reader = read(socketFd, buf, BUFSIZ)) < 0) {
                        perror("error reading stream message");
                } else if (reader == 0) {
                        printf("ending connection from %s\n", connectionIP);
                } else {
                        printf("Client %s sent: %s", connectionIP, buf);
                }
        } while (reader != 0);
        close(socketFd);
}

void
handle6Socket(int s) {
        int socketFd, reader;
        socklen_t socklen;
        const char *connectionIP;

        char addr[INET6_ADDRSTRLEN];
        struct sockaddr_in6 client;

        socklen = sizeof(client);
        if ((socketFd = accept(s, (struct sockaddr *) &client, &socklen)) < 0) {
                perror("accept");
                return;
        }

        if ((connectionIP = inet_ntop(PF_INET6, &(client.sin6_addr), addr, INET6_ADDRSTRLEN)) == NULL) {
                perror("inet_ntop");
                return;
        } else {
                printf("client connected from %s\n", connectionIP);
        }

        do {
                char buf[BUFSIZ];
                bzero(buf, sizeof(buf));
                if ((reader = read(socketFd, buf, BUFSIZ)) < 0) {
                        perror("error reading stream message");
                } else if (reader == 0) {
                        printf("ending connection from %s\n", connectionIP);
                } else {
                        printf("Client %s sent: %s", connectionIP, buf);
                }
        } while (reader != 0);
        close(socketFd);
}


int
selectSocket()
{
        int socket;

        if (ipv == 4) {
                socket = open4Socket();
        } else {
                socket = open6Socket();
        }

        for (;;) {
                fd_set ready;
                struct timeval to;

                FD_ZERO(&ready);
                FD_SET(socket, &ready);
                to.tv_sec = SLEEP;
                to.tv_usec = 0;
                if (select(socket + 1, &ready, 0, 0, &to) < 0) {
                        perror("select");
                        continue;
                }
                if (FD_ISSET(socket, &ready)) {
                        if (ipv == 4) {
                                handle4Socket(socket);
                        } else {
                                handle6Socket(socket);
                        }
                }
        }
}

void
debugSocket()
{
        int socket;

        if (ipv == 4) {
                socket = open4Socket();
        } else {
                socket = open6Socket();
        }

        for (;;) {
                if (ipv == 4) {
                        handle4Socket(socket);
                } else {
                        socket = handle6Socket(socket);
                }
        }
}
