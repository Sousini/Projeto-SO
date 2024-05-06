#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "defs.h"

#define LOG_FILE "task_log.txt"

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

    PROCESS_REQUESTS *pr = init_process_requests(parallel_tasks);

    unlink(ORCHESTRATOR);
    make_fifo(ORCHESTRATOR);

    int fd_read, fd_write;
    open_fifo(&fd_read, ORCHESTRATOR, O_RDWR);
    open_fifo(&fd_write, ORCHESTRATOR, O_WRONLY);

    while (1)
    {

        Msg msg;
        ssize_t read_bytes;
        read_bytes = read(fd_read, &msg, sizeof(Msg));

        if (read_bytes > 0)
        {
            if (strcmp(msg.program_and_args, "status") == 0)
            {
                process_status_request(pr, output_folder);
            }
            else if (msg.status == NEW)
            {
                msg.id = pr->count + 1;
                int pid = fork();
                if (pid == 0)
                {
                    execute_task(&msg, output_folder);
                    _exit(0);
                }
                msg.status = RUNNING;
                add_request(pr, msg);
            }
            else if (msg.status == WAIT) {
                wait(NULL);
                change_process_status(pr,msg.id,DONE);

                 // Abrir ou criar o arquivo de log para registro
                int log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666);
                if (log_fd == -1)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }

                // Escrever no arquivo o identificador da tarefa e seu tempo de execução
                char log_msg[50];
                sprintf(log_msg, "Task ID: %d, Execution Time: %.2f ms\n", msg.id, msg.execution_time);
                write(log_fd, log_msg, strlen(log_msg));

                // Fechar o arquivo
                close(log_fd);
            }
        }
    }

    close(fd_read);
    close(fd_write);
    unlink(ORCHESTRATOR);

    free_process_requests(pr);
    return 0;
}
