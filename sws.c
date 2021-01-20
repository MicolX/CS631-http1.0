/*
 * sws.c
 * Serves as a simple web server (sws) to handle HEAD and GET requests on the UNIX file system hierarchy.
 * Also supports features such as CGIs and user directories.
 *
 * Mingyao Xiong
 * Liam Brew
 *
 * Based on material from Stevens Institute of Technology's
 * CS 631 APUE during the Fall 2020 semester.
 */

#include "sws.h"

int c_opt, d_opt, h_opt, i_opt, l_opt, p_opt, logFd = STDOUT_FILENO, port = 8080, ipv = 6;
char *dir, *cgiDir, *addr, *file, *ipAddr;

int main(int, char **);

/*
 * Determines if a directory pathing is valid/
 */
int testDir(char *dir)
{
	errno = 0;
	DIR *dirTest;
	dirTest = opendir(dir);
	if (dirTest)
	{
		closedir(dirTest);
		return 0;
	}
	else
	{
		return -1;
	}
}


/*
 * Parses and validations options. Starts the server depending on the input instructions.
 */
int main(int argc, char **argv)
{
	char opt;
	ipAddr = NULL;

	while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1)
	{
		switch (opt)
		{
		case 'c':
			if (testDir(optarg) != 0)
			{
				err(EXIT_FAILURE, "Failed to open CGI directory");
			}
			cgiDir = optarg;
			c_opt = 1;
			break;

		case 'd':
			d_opt = 1;
			break;

		case 'h':
			h_opt = 1;
			printf("Usage: %s [-c dir] [-dh] [-i address] [-l file] [-p port] dir\n", argv[0]);
			exit(EXIT_SUCCESS);

		case 'i':
			if ((ipv = verifyIp(optarg)) == -1)
			{
				fprintf(stderr, "%s: invalid IP Address '%s'\n", argv[0], optarg);
				exit(EXIT_FAILURE);
			}
			ipAddr = strdup(optarg);
			i_opt = 1;
			break;

		case 'l':
			file = optarg;
			if ((logFd = open(file, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU)) == -1)
			{
				if ((logFd = openat(logFd, basename(file), O_WRONLY | O_CREAT | O_APPEND, S_IRWXU)) < 0)
				{
					fprintf(stderr, "%s: invalid logging file '%s': %s\n", argv[0], optarg, strerror(errno));
					exit(EXIT_FAILURE);
				}
				fprintf(stderr, "%s: invalid logging file '%s': %s\n", argv[0], optarg, strerror(errno));
				exit(EXIT_FAILURE);
			}
			l_opt = 1;
			break;

		case 'p':
			errno = 0;
			port = atoi(optarg);
			if (errno != 0)
			{
				fprintf(stderr, "%s: invalid port '%s'\n", argv[0], optarg);
				exit(EXIT_FAILURE);
			}

			if (port < 1024 || port > 49151)
			{
				fprintf(stderr, "Invalid port number, can only between 1024~49151.\n");
				exit(EXIT_FAILURE);
			}

			p_opt = 1;
			break;

		default:
			fprintf(stderr, "%s: invalid option '%c'\n", argv[0], opt);
			exit(EXIT_FAILURE);
		}
	}

	dir = argv[optind];

	if (dir == NULL)
	{
		/* Assume this current directory */
		dir = ".";
	}

	if ((d_opt == 0) && (l_opt == 0))
	{
		/* No logging, redirect logFd to /dev/null */
		if ((logFd = open("/dev/null", O_WRONLY)) == -1)
		{
			perror("redirect log to /dev/null");
			exit(EXIT_FAILURE);
		}
	}

	if (testDir(dir) != 0)
	{
		/* Checking dir right before the networking code starts (moved from in opt loop) */
		err(EXIT_FAILURE, "Failed to open root directory: %s\n", strerror(errno));
	}

	if (chdir(dir) != 0)
	{
		err(EXIT_FAILURE, "Failed to chdir: %s\n", strerror(errno));
	}

	//	if (chroot(dir) != 0) {
	//		perror("chroot");
	//		exit(EXIT_FAILURE);
	//	}

	openlog(argv[0], LOG_PID, 0); /* Opens system logging to track server errors */

	startServer();

	return EXIT_SUCCESS;
}
