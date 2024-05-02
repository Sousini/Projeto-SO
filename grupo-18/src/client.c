#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"

int main(int argc, char *argv[])
{

    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s execute time -u/-p \"program [args]\"\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *execution_type = argv[1];
    int time = atoi(argv[2]);

    if (strcmp(execution_type, "execute") != 0) {
        fprintf(stderr, "Invalid command\n");
        exit(EXIT_FAILURE);
    }

    Msg msg;
    msg.occurrences = 0;
    msg.time_estimated = time;

    if (strcmp(argv[3], "-u") == 0) {
        if (argc < 5) {
            fprintf(stderr, "Usage: %s execute time -u \"program [args]\"\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        strncpy(msg.program_and_args, argv[4], sizeof(msg.program_and_args));
    } else if (strcmp(argv[3], "-p") == 0) {
        if (argc < 5) {
            fprintf(stderr, "Usage: %s execute time -p \"program [args] | program [args] | ...\"\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        strncpy(msg.program_and_args, argv[4], sizeof(msg.program_and_args));
    } else {
        fprintf(stderr, "Invalid option\n");
        exit(EXIT_FAILURE);
    }

    make_fifo(ORCHESTRATOR);
    int fd_write;
    open_fifo(&fd_write, ORCHESTRATOR, O_WRONLY);

    ssize_t written_bytes = write(fd_write, &msg, sizeof(Msg));

    if (written_bytes != sizeof(Msg))
    {
        perror("Write Error");
        close(fd_write);
        exit(EXIT_FAILURE);
    }

    close(fd_write);

    return 0;
}
