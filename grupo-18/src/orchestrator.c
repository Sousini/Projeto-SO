#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "defs.h"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s output_folder parallel-tasks sched-policy\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *output_folder = argv[1];
    int parallel_tasks = atoi(argv[2]);
    char *sched_policy = argv[3];

    PROCESS_REQUESTS *pr = init_process_requests(REQUESTS);

    unlink(ORCHESTRATOR);
    make_fifo(ORCHESTRATOR);

    int fd_read, fd_write;
    open_fifo(&fd_read, ORCHESTRATOR, O_RDWR);
    open_fifo(&fd_write, ORCHESTRATOR, O_WRONLY);

    while (1) {
    schedule_tasks(pr, output_folder, parallel_tasks);

    Msg msg;
    ssize_t read_bytes;
    read_bytes = read(fd_read, &msg, sizeof(Msg));

    if (read_bytes > 0) {
        if (strcmp(msg.program_and_args, "status") == 0) {
            process_status_request(pr, output_folder);
        } else {
            msg.id = pr->count + 1;
            add_request(pr, msg);
            execute_task(&msg, output_folder);
            remove_request(pr, 0);
        }
    }
}

    close(fd_read);
    close(fd_write);
    unlink(ORCHESTRATOR);

    free_process_requests(pr);
    return 0;
}
