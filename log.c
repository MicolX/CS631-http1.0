#include "log.h"

void
writeLog(char *str)
{
	//TODO: add labeling
        int written = write(logFd, str, strlen(str));
        if (written < 0){
                perror("write");
		exit(EXIT_FAILURE);
         }
	
}
