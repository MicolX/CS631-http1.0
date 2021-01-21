/*
 * log.c
 * Logs the requests made to the sws server to a record keeping
 * file. **Note**: logging that deals with the running of the server
 * itself, such as encountered errors, is handled via syslog(3).
 *
 * Mingyao Xiong
 * Liam Brew
 *
 * Based on material from Stevens Institute of Technology's
 * CS 631 APUE during the Fall 2020 semester.
 */

#include "log.h"

#define CODELEN 4

/*
 * Logs requests made to the server to the designated logging file. This file is specified as a
 * parameter of the 'l' option on server startup and is validated/opened in sws.c
 */
int writeLog(const char *rip, struct tm *time, char *firstLine, const char *status, long long contentLength)
{
	char timeBuf[TIME_STR_MAX], statusCode[CODELEN];

	(void)strlcpy(statusCode, status, CODELEN);

	(void)strftime(timeBuf, TIME_STR_MAX, "%Y-%m-%dT%H:%M:%SZ", time);

	if (firstLine[strlen(firstLine) - 1] == '\n')
	{
		firstLine[strlen(firstLine) - 1] = '\0';
	}

	int len = strlen(rip) + sizeof(timeBuf) + strlen(firstLine) + strlen(statusCode) + sizeof(contentLength) + 1;
	char *contentBuf = malloc(len);
	
	if (contentBuf == NULL)
	{
		syslog(LOG_INFO, "Error allocating memory for content length - string cast");
		return -1;
	}

	if (snprintf(contentBuf, len, "%s %s %s %s %lld\n", rip, timeBuf, firstLine, statusCode, contentLength) < 0)
	{ /* Gets rid of newline char */
		syslog(LOG_INFO, "Failed at snprintf()");
		return -1;
	}

	if (write(logFd, contentBuf, strlen(contentBuf)) < 0)
	{
		syslog(LOG_INFO, "Error writing IP to log");
		return -1;
	}

	(void)free(contentBuf);

	return 0;
}
