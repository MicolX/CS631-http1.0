#include "log.h"



int
writeLog(const char *rip, struct tm *time, char *firstLine, const char *status, long long contentLength)
{
        char timeBuf[TIME_STR_MAX];
        snprintf(timeBuf, TIME_STR_MAX, "%d-%d-%dT%d:%d:%dZ", (time->tm_year + 1900), time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);


        char *contentBuf = malloc(sizeof(contentLength));
        if (contentBuf == NULL) {
                syslog(LOG_INFO, "Error allocating memory for content length - string cast");
                return -1;
        }

        if (snprintf(contentBuf, strlen(contentBuf), "%lld", contentLength) < 0) { /* Gets rid of newline char */

                syslog(LOG_INFO, "Error converting content length to string");
                return -1;
        }

//	char *statusBuf = malloc(sizeof(status) - 1);
//	if (statusBuf == NULL) {
//		syslog(LOG_INFO, "Error allocating memory for status buffer");
//		return -1;
//	}
//
//	snprintf(statusBuf, strlen(statusBuf), "%s", status);
//
//        char message[100000];     /* Extra to account for spaces and ' */
//        if (message == NULL) {
//                syslog(LOG_INFO, "Error allocating memory for msg");
//                return -1;
//        }
//
//        snprintf(message, 10000, "%s %s '%s' %s %lld\n", rip, timeBuf, firstLine, statusBuf, contentLength);
//
//
//        if (write(logFd, message, strlen(message)) < 0) {
//                perror("write");
//		syslog(LOG_INFO, "Error writing message");
//                return -1;
//        }


        if (write(logFd, rip, sizeof rip) < 0) {
                perror("Error writing IP to log");
                syslog(LOG_INFO, "Error writing IP to log");
                return -1;
        }

        if (write(logFd, " ", 1) < 0) {
                perror("Error writing space to log");
                syslog(LOG_INFO, "Error writing space to log");
                return -1;
        }

        if (write(logFd, timeBuf, strlen(timeBuf)) < 0) {
                perror("Error writing time to log");
                syslog(LOG_INFO, "Error writing time to log");
                return -1;
        }

        if (write(logFd, " '", 2) < 0) {
                perror("Error writing open quote to log");
                syslog(LOG_INFO, "Error writing open quote to log");
                return -1;
        }

        if (write(logFd, firstLine, strlen(firstLine) -1) < 0) {
                perror("Error writing first line to log");
                syslog(LOG_INFO, "Error writing first line to log");
                return -1;
        }

        if (write(logFd, "' ", 2) < 0) {
                perror("Error writing close quote to log");
                syslog(LOG_INFO, "Error writing close quote to log");
                return -1;
        }


        if (write(logFd, status, 3) < 0) {
                perror("Error writing status to log");
                syslog(LOG_INFO, "Error writing status to log");
                return -1;
        }

        if (write(logFd, " ", 1) < 0) {
                perror("Error writing space to log");
                syslog(LOG_INFO, "Error writing space to log");
                return -1;
        }

        if (write(logFd, contentBuf, strlen(contentBuf)) < 0) {
                perror("Error writing content length to log");
                syslog(LOG_INFO, "Error writing content length to log");
                return -1;
        }

        if (write(logFd, "\n", 1) < 0) {
                perror("Error writing new line to log");
                syslog(LOG_INFO, "Error writing new line to log");
                return -1;
        }

        return 0;
}
