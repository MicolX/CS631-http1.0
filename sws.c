#include "sws.h"

int c_opt, d_opt, h_opt, i_opt, l_opt, p_opt, port = 8080, logFd;
char *dir, *addr, *file;


int
main(int argc, char **argv)
{
        int hasopt = 0;
        char opt;

        while (optind < argc) {
                if ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
                        switch (opt) {
                                case 'c':
                                        if (optarg == NULL) {
                                                fprintf(stderr, "-%c: missing argument\n", opt);
                                                exit(EXIT_FAILURE);
                                        }
                                        dir = optarg;
                                        c_opt = 1;
                                        hasopt = 1;
                                        break;

                                case 'd':
                                        logFd = 0;
                                        d_opt = 1;
                                        hasopt = 1;
                                        break;

                                case 'h':
                                        h_opt = 1;
                                        printf("Usage: %s [-c dir] [-dh] [-i address] [-l file] [-p port]\n", argv[0]);
                                        exit(EXIT_SUCCESS);

                                case 'i':
                                        addr = optarg;  //TODO: validate if proper IPv4/IPv6 using inet_pton(3)
                                        i_opt = 1;
                                        hasopt = 1;
                                        break;

                                case 'l':
                                        if (optarg == NULL) {
                                                fprintf(stderr, "-%c: missing argument\n", opt);
                                                exit(EXIT_FAILURE);
                                        }
                                        file = optarg;
                                        l_opt = 1;
                                        hasopt = 1;
                                        break;

                                case 'p':
                                        if (optarg == NULL) {
                                                fprintf(stderr, "-%c: missing argument\n", opt);
                                                exit(EXIT_FAILURE);
                                        }
					char *temp = NULL;
                                        port = (int)strtol(optarg, &temp, 10);
                                        errno = 0;
					if ((temp == optarg) || (errno != 0)) {
                                                fprintf(stderr, "%s: invalid port '%s'\n", argv[0], optarg);
                                                exit(EXIT_FAILURE);
                                        }
                                        p_opt = 1;
                                        hasopt = 1;
                                        break;

                                default:
                                        fprintf(stderr, "%s: invalid option '%c'\n", argv[0], opt);
                                        exit(EXIT_FAILURE);
                        }
                }
        }

	if (hasopt) {
		;
	}

        /* Daemon Process */
        if (d_opt == 0) {
                // Step 1: clear environment
		environ = NULL;

                // Step 2: fork off parent process
                pid_t pid;
                pid = fork();

                if (pid < 0) {  // error
                        perror("error in daemon process");
                        exit(EXIT_FAILURE);
                }

                if (pid > 0) {  // parent
                        exit(EXIT_SUCCESS);
                }

                //TODO: signal handlers (see HW #2)

                // Step 3: change file mode mask (umask)
                umask(0);

                // Step 4: create unique Session ID (SID)
                if (setsid() < 0) {     // child inherits group leadership
                        perror("error promoting child to group leader");
                        _exit(errno);
                }

                // Step 5: change cwd to a safe place
                if (chdir("/") != 0) {     // what counts as 'safe'?
                        perror("error changing directory to safe place");
                        _exit(errno);
                }

                // Step 6: close/redirect standard file descriptors
                //TODO: which to do? want to incorporate later error handling and output...
                if (close(1) != 0) {      //stdin should be closed regardless
                        perror("error closing stdin");
                        _exit(errno);
                }

                // Step 7: open any logs for writing
                if (l_opt == 1) {       // opens specified file for logging as per l_opt
                        //TODO: investigate how to redirect syslog(3) to a file
                        if ((logFd = open(file, O_WRONLY | O_CREAT | O_TRUNC | O_REGULAR, S_IRWXU)) < 0) {
                                perror("error opening log file");
                                _exit(errno);
                        }
                }

                // Step 8: actual code
        }






}

