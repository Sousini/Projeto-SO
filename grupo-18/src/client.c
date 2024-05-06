#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "defs.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s execute time -u/-p \"program [args]\" OR %s status\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "status") == 0) {
        // Solicitar status ao servidor
        request_status(ORCHESTRATOR);
    } else {
        // Se não for um pedido de status, continue como antes
        if (argc < 4) {
            fprintf(stderr, "Usage: %s execute time -u \"program [args]\" OR %s execute time -p \"program [args] | program [args] | ...\"\n", argv[0], argv[0]);
            exit(EXIT_FAILURE);
        }

        char *execution_type = argv[1];
        int time = atoi(argv[2]);

        Msg msg;
        msg.occurrences = 0;
        msg.time_estimated = time;
        msg.status = NEW;
        
        if (strcmp(execution_type, "execute") != 0) {
            fprintf(stderr, "Invalid command\n");
            exit(EXIT_FAILURE);
        }

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
        sprintf(task_sent_msg, "Task %d sent\n", msg.id);
        write(STDOUT_FILENO, task_sent_msg, strlen(task_sent_msg));

        close(fd_write);
    }

    return 0;
}
