#include "log.h"

void
writeLog(Log *log)
{
	char *msg = malloc(strlen(log->remoteIp) + sizeof log->time + strlen(log->firstLine) + strlen(log->status) + sizeof log->contentLength);
	if (msg == NULL) {
                perror("logging message malloc");
                exit(EXIT_FAILURE);
	}

        if (snprintf(msg, strlen(msg), "%s %s '%s' %s %d\n", log->remoteIp, log->time, log->firstLine, log->status, log->contentLength) < 0) {
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
