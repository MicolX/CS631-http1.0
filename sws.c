#include "sws.h"

int c_opt, d_opt, h_opt, i_opt, l_opt, p_opt, logFd = STDOUT_FILENO, port = 8080, ipv = 6, rootfd;      
char *dir, *cgiDir, *addr, *file, *ipAddr;


int
testDir(char *dir)
{
        DIR *dirTest;
        dirTest = opendir(dir);
        if (dirTest) {
                closedir(dirTest);
                return EXIT_SUCCESS;
        } else {
                fprintf(stderr, "invalid directory '%s'\n", dir);
                return EXIT_FAILURE;
        }
}

int
main(int argc, char **argv)
{
        char *temp = NULL;
	char opt;

        while (optind < argc) {
                if ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
                        switch (opt) {
                                case 'c':
                                        cgiDir = optarg;


                                        testDir(cgiDir);  //TODO: move this to the CGI code base

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
                                        ipv = verifyIp(optarg);
                                        if (ipv == -1) {
                                                fprintf(stderr, "%s: invalid IP Address '%s'\n", argv[0], optarg);
                                                exit(EXIT_FAILURE);
                                        }
                                        i_opt = 1;
                                        break;

                                case 'l':
                                        file = optarg;
                                        if((logFd = open(file, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU)) == -1) {
                                                if ((logFd = openat(logFd, basename(file), O_WRONLY  | O_CREAT | O_APPEND, S_IRWXU)) < 0) {
                                                        fprintf(stderr, "%s: invalid logging file '%s': %s\n", argv[0], optarg, strerror(errno));
                                                        exit(EXIT_FAILURE);
                                                }
                                                fprintf(stderr, "%s: invalid logging file '%s': %s\n", argv[0], optarg, strerror(errno));
                                                exit(EXIT_FAILURE);
                                        }
					l_opt = 1;
					break;

                                case 'p':
                                        port = (int)strtol(optarg, &temp, 10);
                                        errno = 0;
					if (errno != 0) {
                                                fprintf(stderr, "%s: invalid port '%s'\n", argv[0], optarg);
                                                exit(EXIT_FAILURE);
                                        }
                                        p_opt = 1;
                                        break;

                                default:
                                        fprintf(stderr, "%s: invalid option '%c'\n", argv[0], opt);
                                        exit(EXIT_FAILURE);
                        }
                } else {
                	dir = argv[optind];

			if (dir == NULL) {	/* Assume this current directory */
				dir = ".";
			}
			optind++;
       		}
	}

	if ((d_opt == 0) && (l_opt == 0)) {	/* No logging, redirect logFd to /dev/null */
		if ((logFd = open("/dev/null", O_WRONLY)) == -1) {
			perror("redirect log to /dev/null");
			exit(EXIT_FAILURE);
		}
	}

        if (testDir(dir) != EXIT_SUCCESS) { /* Checking dir right before the networking code starts (moved from in opt loop) */
                exit(EXIT_FAILURE);
        }

      //  if (chdir(dir) != 0) {
      //          perror("chdir");
      //          exit(EXIT_FAILURE);
      //  }

        openlog(argv[0], LOG_PID, 0);       /* Opens system logging to track server errors */

        if (d_opt == 0) {
                if (daemon(0, 1) == -1) {
                        perror("daemon");
                        exit(EXIT_FAILURE);
                }

		printf("Directory: %s\n", dir);
        }

        startServer();
}


