#include "sws.h"

int c_opt, d_opt, h_opt, i_opt, l_opt, p_opt, logFd, port = 8080, ipv = 6;      
char *dir, *cgiDir, *addr, *file, *ipAddr;


int
main(int argc, char **argv)
{
        DIR *dirTest;
        char *temp = NULL;
	char opt;

        while (optind < argc) {
                if ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
                        switch (opt) {
                                case 'c':
                                        cgiDir = optarg;
                                        dirTest = opendir(cgiDir);
                                        if (dirTest) {
                                                closedir(dirTest);
						dirTest = NULL;
                                        } else {
                                                fprintf(stderr, "%s: invalid directory '%s'\n", argv[0], cgiDir);
                                                exit(EXIT_FAILURE);
                                        }
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
			
			/* Check if dir is valid */
                	dirTest = opendir(dir);
                	if (dirTest) {
                  		closedir(dirTest);
                	} else {
                 	 	fprintf(stderr, "%s: invalid directory '%s'\n", argv[0], dir);
                    		exit(EXIT_FAILURE);
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

        if (d_opt == 1) {
		debugSocket();
        } else {
                daemonize();
        }
	
	//printf("Server started on Port #%d\n", ntohs(port));

}


