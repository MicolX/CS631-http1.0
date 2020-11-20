#include "process.h"

void
daemonize(void)
{
	int pout[2], perr[2];
        pid_t pid;

	if (pipe(pout) != 0) {
		perror("stdout pipe");
		exit(EXIT_FAILURE);
	}

	if (pipe(perr) != 0) {
		perror("stderr pipe");
		exit(EXIT_FAILURE);
	}

        pid = fork();

        if (pid < 0){
                perror("fork");
                exit(EXIT_FAILURE);
        }

        if (pid > 0) {
		printf("Server running on PID #%d\n", pid); 
                exit(EXIT_SUCCESS);
        }

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


	if (close(pout[0]) == -1) {
		_exit(errno);
	}

	if (close(perr[0]) == -1) {
		_exit(errno);
	}

	if (dup2(pout[1], 1) == -1) {
		_exit(errno);
	}
	
	if (dup2(perr[1], 2) == -1) {
		_exit(errno);
	}

	if (close(pout[1]) == -1) {
		_exit(errno);
	}

	if (close(perr[1]) == -1) {
		_exit(errno);
	}	
	
	for (;;) {
		// selectSocket();
	}

}
