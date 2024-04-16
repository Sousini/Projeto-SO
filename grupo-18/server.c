#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"

int count_needle(const char *haystack, const char *needle)
{
    int count = 0;
    size_t needle_len = strlen(needle);
    const char *tmp = haystack;

    while ((tmp = strstr(tmp, needle)))
    {
        count++;
        tmp += needle_len;
    }

    return count;
}

int main(int argc, char *argv[])
{

    int fd_ret;

    if (mkfifo("FIFO", 0666) == -1)
    {
        perror("create error");
        exit(1);
    }

    int fildes = open("FIFO", O_RDWR);
    if (fildes == -1)
    {
        perror("open error");
        exit(1);
    }

    Msg new;
    ssize_t read_bytes;
    char buffer[1024];
    while (1)
    {
        read_bytes = read(fildes, &new, sizeof(Msg));
        if (read_bytes > 0)
        {
            int occurrences = count_needle(new.program_and_args, "needle");
            new.occurrences = occurrences;
            snprintf(buffer, 1024, "FIFO, %d", new.pid);
            fd_ret = open(buffer, O_WRONLY);
            write(fd_ret, &new, sizeof(Msg));
            close(fd_ret);
        }
    }
    close(fildes);
    unlink("FIFO");

    return 0;
}