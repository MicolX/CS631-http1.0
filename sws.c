#include "sws.h"

int port = 8080;
char *dir, *addr, *file;


int
main(int argc, char **argv)
{
        int hasopt = 0, previous, optchar;
        char opt;
        options *opts;

        if ((hasopt = (options *) malloc(sizeof(options))) == NULL) {
              fprintf(stderr, "error allocating memory for options\n");
        }

        while (optind < argc) {
                if ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
                        switch (opt) {
                                case 'c':
                                        if (optarg == NULL) {
                                                fprintf(stderr, "-%c: missing argument\n", opt);
                                                exit(EXIT_FAILURE);
                                        }
                                        dir = optarg;
                                        opts->c_opt = 1;
                                        hasopt = 1;
                                        break;

                                case 'd':
                                        opts->d_opt = 1;
                                        hasopt = 1;
                                        break;

                                case 'h':
                                        printf("Usage: %s [-c dir] [-dh] [-i address] [-l file] [-p port]\n", argv[0]);
                                        exit(EXIT_SUCCESS);

                                case 'i':
                                        addr = optarg;  //TODO: validate if proper IPv4/IPv6 using inet_pton(3)
                                        opts->i_opt = 1;
                                        hasopt = 1;
                                        break;

                                case 'l':
                                        if (optarg == NULL) {
                                                fprintf(stderr, "-%c: missing argument\n", opt);
                                                exit(EXIT_FAILURE);
                                        }
                                        file = optarg;
                                        opts->l_opt = 1;
                                        hasopt = 1;
                                        break;

                                case 'p':
                                        if (optarg == NULL) {
                                                fprintf(stderr, "-%c: missing argument\n", opt);
                                                exit(EXIT_FAILURE);
                                        }
                                        char *temp;
                                        port = (int)strtol(optarg);
                                        if (temp != '\0') {
                                                fprintf(stderr, "%s: invalid port '%c'\n", argv[0], optarg);
                                                exit(EXIT_FAILURE);
                                        }
                                        opts->p_opt = 1;
                                        hasopt = 1;
                                        break;

                                default:
                                        fprintf(stderr, "invalid option '%c'\n", argv[0], opt);
                                        exit(EXIT_FAILURE);
                        }
                }
        }




}


