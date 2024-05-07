#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "defs.h"

void print_usage(char *argv[])
{
    char usage_msg[200];
    sprintf(usage_msg, "Usage: %s execute time -u/-p \"program [args]\" OR %s status\n", argv[0], argv[0]);
    write(STDERR_FILENO, usage_msg, strlen(usage_msg));
    exit(EXIT_FAILURE);
}

void print_invalid(char *message)
{
    write(STDERR_FILENO, message, strlen(message));
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv);
    }

    if (strcmp(argv[1], "status") == 0)
    {
        // Solicitar status ao servidor
        char buffer[1024];
        snprintf(buffer, 1024, "tmp/FIFO_%d", getpid());
        make_fifo(buffer);
        request_status(getpid());
    }
    else
    {
        // Se não for um pedido de status, continue como antes
        if (argc < 4)
        {
            print_usage(argv);
        }

        char *execution_type = argv[1];
        int time = atoi(argv[2]);

        Msg msg;
        msg.occurrences = 0;
        msg.time_estimated = time;
        msg.status = NEW;
        msg.pid = getpid();

        if (strcmp(execution_type, "execute") != 0)
        {
            print_invalid("Invalid command\n");
        }

        if (strcmp(argv[3], "-p") == 0 || strcmp(argv[3], "-p") == 0)
        {
            if (argc < 5)
            {
                print_invalid("Functionality not implemented\n");
            }
        }
        else if (strcmp(argv[3], "-u") == 0)
        {
            if (argc < 5)
            {
                print_usage(argv);
            }
            strncpy(msg.program_and_args, argv[4], sizeof(msg.program_and_args));
        }
        else
        {
            print_invalid("Invalid option\n");
        }

        // Enviar pedido de execução para o servidor
        int fd_write;
        open_fifo(&fd_write, ORCHESTRATOR, O_WRONLY);

        ssize_t written_bytes = write(fd_write, &msg, sizeof(Msg));

        if (written_bytes != sizeof(Msg))
        {
            perror("Write Error");
            close(fd_write);
            exit(EXIT_FAILURE);
        }

        char task_sent_msg[50];
        sprintf(task_sent_msg, "Task sent\n");
        write(STDOUT_FILENO, task_sent_msg, strlen(task_sent_msg));
        close(fd_write);
    }

    return 0;
}
