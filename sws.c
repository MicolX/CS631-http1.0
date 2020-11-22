#include "sws.h"

int c_opt, d_opt, h_opt, i_opt, l_opt, p_opt, port = 8080, logFd = 1, ipv = 6;      //TODO: change logfd from stdout and default IPv
char *dir, *cgiDir, *addr, *file, *ipAddr;


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
                                        c_opt = 1;
                                        break;

                                case 'd':
                                        logFd = 1;
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
                }
        }

        if (d_opt == 1) {
                debugSocket();
        } else {
                daemonize();
        }







        if (close(logFd) != 0){
                perror("close");
                exit(EXIT_FAILURE);
        }

}


