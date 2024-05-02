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

    int tasks_running = 0;
    while (1)
    {
        if (tasks_running < parallel_tasks)
        {
            Msg msg;
            ssize_t read_bytes;
            read_bytes = read(fd_read, &msg, sizeof(Msg));
            if (read_bytes > 0)
            {
                msg.id = pr->count + 1;
                add_request(pr, msg);
                execute_task(&msg, output_folder);
                remove_request(pr, 0);
                tasks_running++;
            }
        }
        else
        {
            char response[] = "Server busy. Please try again later.\n";
            write(fd_write, response, strlen(response));
        }
    }

    close(fd_read);
    close(fd_write);
    unlink(ORCHESTRATOR);

    free_process_requests(pr);
    return 0;
}
