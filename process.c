#include "process.h"

void
daemonize(void)
{
        environ = NULL;

        pid_t pid;

        pid = fork();

        if (pid < 0){
                perror("fork");
                exit(EXIT_FAILURE);
        }

        if (pid > 0){
                printf("successful fork \n");
                exit(EXIT_SUCCESS);
        }

        /* On success: The child process becomes session leader */
        if (setsid() < 0){
                perror("setsid");
                _exit(EXIT_FAILURE);
        }

        //TODO: signal handler */
        signal(SIGCHLD, SIG_IGN);
        signal(SIGHUP, SIG_IGN);

        umask(0);

        if (chdir("/") != 0) {     // based on CWD
                perror("error changing directory to safe place");
                _exit(errno);
        }

        /* Close file descriptors */
        if (close(0) == -1) {
                _exit(errno);
        }

        if (close(1) == -1) {
                _exit(errno);
        }

        if (close(2) == -1) {
                _exit(errno);
        }

        writeLog("success");
}