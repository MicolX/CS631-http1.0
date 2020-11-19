#include "log.h"

blksize_t
getBlkSize(int fd)
{
        struct stat logStat;

        if (fstat(logFd, &logStat) != 0) {
                perror("stat");
                exit(EXIT_FAILURE);
        }

        return logStat.st_blksize;
}

void
writeLog(char *str)
{
        int writeBytes;
        int strLen = strlen(str);

        char *buffer;

        blksize_t logSize = getBlkSize(logFd);

        while (writeBytes < strLen) {
                int written = write(logFd, str, logSize);
                if (written < 0){
                        perror("write");
                        exit(EXIT_FAILURE);
                }

                writeBytes = writeBytes + written;

                size_t x = 0;
                for (size_t y = writeBytes; y < strLen - 1; ++y) {
                        buffer[x++] = str[y];
                }
                str = buffer;
        }
}