/*
 * connect.c
 * Handles networking aspect of the sws program.
 *
 * Mingyao Xiong
 * Liam Brew
 *
 * Based on material from Stevens Institute of Technology's
 * CS 631 APUE during the Fall 2020 semester.
 */

#include "connect.h"

#define CGIPREFIX "/cgi-bin"

int domain;

/*
 * Determines if a string represents a valid IP address, and if so what
 * IP version this address is.
 */
int verifyIp(const char *str)
{
	if ((inet_pton(AF_INET, str, &ipAddr)) == 1)
	{
		return 4;
	}
	else if ((inet_pton(AF_INET6, str, &ipAddr)) == 1)
	{
		return 6;
	}
	else
	{
		return -1;
	}
}

/*
 * Establishes a socket connection using the data passed in as options.
 */
int openSocket(void)
{
	int sock;
	void *s;
	socklen_t length, s_size;
	struct sockaddr_storage server;

	if (ipv == 4)
	{
		domain = PF_INET;
	}
	else
	{
		domain = PF_INET6;
	}

	if ((sock = socket(domain, SOCK_STREAM, 0)) < 0)
	{		
		fprintf(stderr, "Error creating IPv%d socket", ipv);
		exit(EXIT_FAILURE);
	}

	if (domain == PF_INET)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)&server;
		sin->sin_family = PF_INET;
		sin->sin_addr.s_addr = ipAddr ? inet_addr(ipAddr) : INADDR_ANY;
		sin->sin_port = htons(port);
		s = sin;
		s_size = sizeof(*sin);
	}
	else
	{
		struct sockaddr_in6 *sin = (struct sockaddr_in6 *)&server;
		sin->sin6_family = PF_INET6;
		if (ipAddr)
		{
			err(EXIT_FAILURE, "Invalid IPv6 address.\n");
		}
		else
		{
			sin->sin6_addr = in6addr_any;
		}
		sin->sin6_port = htons(port);
		s = sin;
		s_size = sizeof(*sin);

		if (i_opt == 1)
		{
			int off = 0;
			if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&off, sizeof(off)) < 0)
			{				
				err(EXIT_FAILURE, "Error setting socket option for both IPv values");
			}
		}
	}

	if (bind(sock, (struct sockaddr *)s, s_size) != 0)
	{		
		err(EXIT_FAILURE, "Error binding socket");
	}

	length = sizeof(server);
	if (getsockname(sock, (struct sockaddr *)&server, &length) != 0)
	{
		err(EXIT_FAILURE, "Error getting socket name");
	}

	if (listen(sock, DEBUG_BACKLOG) < 0)
	{
		err(EXIT_FAILURE, "Error listening on socket");
	}

	if (d_opt == 1)
	{
		printf("Listening on port #%d.\n", port);
	}

	if (d_opt == 0)
	{
		if (daemon(1, 0) == -1)
		{
			err(EXIT_FAILURE, "Failed to daemonize: %s\n", strerror(errno));
		}
	}

	return sock;
}

void handleConnection(int fd)
{
	int rval;
	socklen_t len;
	time_t timer;

	const char *rip;
	char buf[BUFSIZ];
	char claddr[INET6_ADDRSTRLEN];

	struct sockaddr_storage addr;

	struct tm *gtime;

	Request *request = (Request *)malloc(sizeof(Request));
	Response *response = (Response *)malloc(sizeof(Response));

	if (request == NULL)
	{
		if (d_opt)
		{
			perror("Error allocating memory to request structure");
		}
		syslog(LOG_ERR, "Error allocating memory to request structure");
		exit(EXIT_FAILURE);
	}

	if (response == NULL)
	{
		if (d_opt)
		{
			perror("Error allocating memory to response structure");
		}
		syslog(LOG_ERR, "Error allocating memory to response structure");
		exit(EXIT_FAILURE);
	}

	/* No loop here as per instruction connections are terminated after requests are served. */
	bzero(buf, sizeof(buf));
	if ((rval = read(fd, buf, BUFSIZ)) < 0)
	{
		if (d_opt)
		{
			perror("Error reading socket");
		}
		syslog(LOG_ERR, "Error reading socket");
		return;
	}

	if (rval == 0)
	{
		return;
	}

	len = sizeof(addr);
	if (getpeername(fd, (struct sockaddr *)&addr, &len) < 0)
	{
		if (d_opt)
		{
			perror("Error getting peer name");
		}
		syslog(LOG_ERR, "Error getting peer name");
		return;
	}

	if (domain == PF_INET)
	{
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		port = ntohs(s->sin_port);
		rip = inet_ntop(PF_INET, &s->sin_addr, claddr, sizeof(claddr));
	}
	else
	{
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		port = ntohs(s->sin6_port);
		rip = inet_ntop(PF_INET6, &s->sin6_addr, claddr, sizeof(claddr));
	}

	if (rip == NULL)
	{
		if (d_opt)
		{
			perror("inet_ntop error");
		}
		syslog(LOG_ERR, "inet_ntop error");
		rip = "unknown";
	}

	if (time(&timer) == -1)
	{
		if (d_opt)
		{
			perror("Error getting current time");
		}
		syslog(LOG_ERR, "Error getting current time");
		return;
	}

	if ((gtime = gmtime(&timer)) == NULL)
	{
		if (d_opt)
		{
			perror("Error converting current time to GMT time");
		}
		syslog(LOG_ERR, "Error converting current time to GMT time");
		return;
	}

	if (parse(buf, request) == -1)
	{
		if (d_opt)
		{
			perror("Error parsing response");
		}
		syslog(LOG_ERR, "Error parsing response");
	}

	if (strchr(request->uri, (int)'~') != NULL)
	{
		request->hastilde = 1;
		char *old = strdup(request->uri);
		if (userdirhandler(old, request->uri) == -1)
		{
			if (d_opt)
			{
				perror("Error fetching home directory");
			}
			syslog(LOG_ERR, "Error fetching home directory");
			return;
		}
	}

	if (strncmp(request->uri, CGIPREFIX, strlen(CGIPREFIX)) == 0 && c_opt == 1)
	{
		char *uri = request->uri;
		(void)strsep(&uri, "n");

		if (runcgi(fd, uri, cgiDir) == -1)
		{
			if (d_opt)
			{
				perror("Error running cgi");
			}
			syslog(LOG_ERR, "Error running cgi");
		}
	}
	else
	{
		if (respond(dir, request, response) == -1)
		{
			if (d_opt)
			{
				perror("Error composing response");
			}
			syslog(LOG_ERR, "Error composing response");
		}

		if (reply(fd, request, response) == -1)
		{
			if (d_opt)
			{
				perror("Error sending response");
			}
			syslog(LOG_ERR, "Error sending response");
		}
	}

	if (time(&timer) == -1)
	{
		if (d_opt)
		{
			perror("Error getting current time");
		}
		syslog(LOG_INFO, "Error getting current time");
	}

	if (writeLog(rip, gtime, strtok(buf, "\n"), response->status, response->contentlength) == -1)
	{
		if (d_opt)
		{
			perror("Error converting current time to GMT time");
		}
		syslog(LOG_INFO, "Error converting current time to GMT time");
	}

	free(request);
	free(response);
	(void)close(fd);
}

/*
 * Handles connections on a socket by receiving, parsing, and responding
 * to requests.
 */
void handleSocket(int sock)
{
	pid_t pid;
	int sockFd;

	socklen_t size;
	
	struct sockaddr_in6 client;

	size = sizeof(client);
	if ((sockFd = accept(sock, (struct sockaddr *)&client, &size)) < 0)
	{
		if (d_opt)
		{
			perror("Error accepting connection on socket");
		}
		syslog(LOG_ERR, "Error accepting connection on socket");
		exit(EXIT_FAILURE);
	}

	handleConnection(sockFd);
	// if ((pid = fork()) < 0)
	// {
	// 	err(EXIT_FAILURE, "Fork failed");
	// }
	// else if (pid == 0)
	// {
	// 	handleConnection(sockFd);
	// }
}

void reap() 
{
	wait(NULL);
}

/*
 * Handles server startup and loops to continually managa socket connections as they come in.
 */
void startServer(void)
{
	int socket;
	fd_set ready;
	struct timeval to;

	socket = openSocket();

	if (signal(SIGCHLD, reap) == SIG_ERR)
	{
		err(EXIT_FAILURE, "Failed setting SIGCHLD");
	}

	for (;;)
	{
		FD_ZERO(&ready);
		FD_SET(socket, &ready);
		to.tv_sec = SLEEP;
		to.tv_usec = 0;

		if (select(socket + 1, &ready, 0, 0, &to) < 0)
		{
			if (d_opt)
			{
				perror("Error selecting socket");
			}
			syslog(LOG_INFO, "Error selecting socket");
			continue;
		}

		if (FD_ISSET(socket, &ready))
		{
			handleSocket(socket);
		}
	}
}
