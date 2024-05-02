#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include "defs.h"


void make_fifo(const char *fifo_name) {
    mkfifo(fifo_name, 0666);
}

void open_fifo(int *fd, const char *fifo_name, int flags) {
    *fd = open(fifo_name, flags);
    if (*fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
}

void send_client_response(int pid, int id) {
    char response[100];
    sprintf(response, "Response to client with ID %d: Program ID is %d\n", pid, id);
    // Assuming write to stdout
    write(STDOUT_FILENO, response, strlen(response));
}

PROCESS_REQUESTS* init_process_requests() {
    PROCESS_REQUESTS *pr = malloc(sizeof(PROCESS_REQUESTS));
    if (pr) {
        pr->count = 0;
    }
    return pr;
}

void free_process_requests(PROCESS_REQUESTS *pr) {
    // Libere qualquer memória alocada dentro da estrutura pr
    free(pr);
}


bool add_request(PROCESS_REQUESTS *pr, Msg msg) {
    if (pr->count >= REQUESTS) {
        fprintf(stderr, "Error: Maximum requests exceeded\n");
        return false;
    }

    pr->requests[pr->count++] = msg;
    return true;
}

Msg* get_request(PROCESS_REQUESTS *pr, int index) {
    if (index < 0 || index >= pr->count) {
        fprintf(stderr, "Error: Invalid request index\n");
        return NULL;
    }

    return &pr->requests[index];
}

void remove_request(PROCESS_REQUESTS *pr, int index) {
    if (index < 0 || index >= pr->count) {
        fprintf(stderr, "Error: Invalid request index\n");
        return;
    }

    for (int i = index; i < pr->count - 1; i++) {
        pr->requests[i] = pr->requests[i + 1];
    }
    pr->count--;
}

void handle_request(PROCESS_REQUESTS *pr, Msg *msg) {
    // Verificar se o ponteiro pr é válido
    if (!pr) {
        fprintf(stderr, "Error: PROCESS_REQUESTS is not initialized\n");
        return;
    }

    // Adicionar a solicitação à estrutura de solicitações
    if (!add_request(pr, *msg)) {
        fprintf(stderr, "Error: Failed to add request to the PROCESS_REQUESTS\n");
        return;
    }

    // Implemente aqui a lógica para processar a solicitação
    printf("Received request with ID %d\n", msg->id);
    printf("Added to PROCESS_REQUESTS\n");
}

void execute_task(Msg *msg, const char *output_folder) {
    // Crie um nome de arquivo único com base no ID da tarefa
    char filename[20];
    sprintf(filename, "%d.txt", msg->id);

    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/%s", output_folder, filename);

    int fd_output = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_output == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    dup2(fd_output, STDOUT_FILENO);
    dup2(fd_output, STDERR_FILENO);
    close(fd_output);

    printf("Executing: %s\n", msg->program_and_args);
    system(msg->program_and_args);

    printf("Task %d completed\n", msg->id);
}