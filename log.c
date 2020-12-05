#include "log.h"

void
writeLog(Log *log)
{
	char *msg = malloc(strlen(log->remoteIp) + sizeof log->time + strlen(log->firstLine) + sizeof status + sizeof size);
	if (msg == NULL) {
                perror("logging message malloc");
                exit(EXIT_FAILURE);
	}

        if (snprintf(msg, strlen(msg), "%s %Y-%m-%d %H:%M:%S '%s' %s %d\n", log->remoteIp, log->time, log->firstLine, log->status, log->contentLength) < 0) {
                perror("formatting log message");
                exit(EXIT_FAILURE);
        }

        int written = write(logFd, msg, strlen(msg));
        if (written < 0){
                perror("logging message writing");
		exit(EXIT_FAILURE);
        }

        free(msg);
}
