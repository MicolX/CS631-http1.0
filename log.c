#include "log.h"



int
writeLog(Log *log)
{
        char timeBuf[TIME_STR_MAX];

        errno = 0;
        if (strftime(timeBuf, strlen(timeBuf), "%Y-%m-%dT%H:%M:%SZ", log->time) == 0 && errno != 0) {
                syslog(0, "Error creating logging time: %m");
                return -1;
        }

//	char *msg = malloc(strlen(log->remoteIp) + strlen(timeBuf) + strlen(log->firstLine) + strlen(log->status) + sizeof log->contentLength);
	char msg[BUFSIZ];
	if (msg == NULL) {
                syslog(0, "Error allocating memory to log string: %m");
                return -1;
	}

        if (snprintf(msg, strlen(msg), "%s %s '%s' %s %d\n", log->remoteIp, timeBuf, log->firstLine, log->status, log->contentLength) < 0) {
                syslog(0, "Error formatting log string: %m");
                return -1;
        }

        int written = write(logFd, msg, strlen(msg));
        if (written < 0){
                syslog(0, "Error writing log message: %m");
                return -1;
        }

//        free(msg);
        return 0;
}
