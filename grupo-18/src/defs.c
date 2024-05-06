#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "defs.h"

void make_fifo(const char *fifo_name)
{
    mkfifo(fifo_name, 0666);
}

void open_fifo(int *fd, const char *fifo_name, int flags)
{
    *fd = open(fifo_name, flags);
    if (*fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
}

void send_client_response(int pid, int id)
{
    char response[100];
    sprintf(response, "Response to client with ID %d: Program ID is %d\n", pid, id);
    // Escrevendo para STDOUT_FILENO
    write(STDOUT_FILENO, response, strlen(response));
}

PROCESS_REQUESTS *init_process_requests()
{
    PROCESS_REQUESTS *pr = malloc(sizeof(PROCESS_REQUESTS));
    if (pr)
    {
        pr->count = 0;
    }
    return pr;
}

void free_process_requests(PROCESS_REQUESTS *pr)
{
    // Libere qualquer memória alocada dentro da estrutura pr
    free(pr);
}

bool add_request(PROCESS_REQUESTS *pr, Msg msg)
{
    if (pr->count >= REQUESTS)
    {
        char errorMsg[] = "Error: Maximum requests exceeded\n";
        write(STDERR_FILENO, errorMsg, strlen(errorMsg));
        return false;
    }

    pr->requests[pr->count++] = msg;
    return true;
}

Msg *get_request(PROCESS_REQUESTS *pr, int index)
{
    if (index < 0 || index >= pr->count)
    {
        char errorMsg[] = "Error: Invalid request index\n";
        write(STDERR_FILENO, errorMsg, strlen(errorMsg));
        return NULL;
    }

    return &pr->requests[index];
}

void remove_request(PROCESS_REQUESTS *pr, int index)
{
    if (index < 0 || index >= pr->count)
    {
        char errorMsg[] = "Error: Invalid request index\n";
        write(STDERR_FILENO, errorMsg, strlen(errorMsg));
        return;
    }

    for (int i = index; i < pr->count - 1; i++)
    {
        pr->requests[i] = pr->requests[i + 1];
    }
    pr->count--;
}

void handle_request(PROCESS_REQUESTS *pr, Msg *msg)
{
    // Verificar se o ponteiro pr é válido
    if (!pr)
    {
        char errorMsg[] = "Error: PROCESS_REQUESTS is not initialized\n";
        write(STDERR_FILENO, errorMsg, strlen(errorMsg));
        return;
    }

    // Adicionar a solicitação à estrutura de solicitações
    if (!add_request(pr, *msg))
    {
        char errorMsg[] = "Error: Failed to add request to the PROCESS_REQUESTS\n";
        write(STDERR_FILENO, errorMsg, strlen(errorMsg));
        return;
    }

    // Implemente aqui a lógica para processar a solicitação
    char receivedMsg[100];
    sprintf(receivedMsg, "Received request with ID %d\n", msg->id);
    write(STDOUT_FILENO, receivedMsg, strlen(receivedMsg));
    char addedMsg[] = "Added to PROCESS_REQUESTS\n";
    write(STDOUT_FILENO, addedMsg, strlen(addedMsg));
}

void execute_task(Msg *msg, const char *output_folder)
{
    struct timeval start_time, end_time;
    double execution_time;

    // Obter o tempo de início
    gettimeofday(&start_time, NULL);

    // Criar um nome de arquivo único com base no ID da tarefa
    char filename[20];
    sprintf(filename, "%d.txt", msg->id);

    char filepath[100];
    snprintf(filepath, sizeof(filepath), "%s/%s", output_folder, filename);

    int fd_output = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_output == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    int pid_fork = fork();
    if (pid_fork == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid_fork == 0)
    {
        // Processo filho
        dup2(fd_output, STDOUT_FILENO);
        dup2(fd_output, STDERR_FILENO);
        close(fd_output);
        exec_command(msg->program_and_args);
        _exit(EXIT_SUCCESS);
    }
    else
    {
        // Processo pai
        waitpid(pid_fork, NULL, 0);
        close(fd_output); // Fechar após o término do processo filho
    }

    // Obter o tempo de término
    gettimeofday(&end_time, NULL);

    // Calcular o tempo de execução em milissegundos
    execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0;    // segundos para milissegundos
    execution_time += (end_time.tv_usec - start_time.tv_usec) / 1000.0; // microssegundos para milissegundos

    // Atualizar o tempo real de execução na estrutura da tarefa
    msg->execution_time = execution_time;
    msg->status = WAIT; // Definir status como WAIT após a execução

    // Abra o FIFO e escreva a mensagem
    int fd_fifo = open(ORCHESTRATOR, O_WRONLY);
    if (fd_fifo == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Escrever a mensagem para o FIFO
    if (write(fd_fifo, msg, sizeof(Msg)) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    close(fd_fifo);

    char taskExecutedMsg[100];
    sprintf(taskExecutedMsg, "Task %d executed. Execution time: %.2f ms\n", msg->id, msg->execution_time);
    write(STDOUT_FILENO, taskExecutedMsg, strlen(taskExecutedMsg));
}

void schedule_tasks(PROCESS_REQUESTS *pr, const char *output_folder, int parallel_tasks)
{
    int tasks_to_execute = pr->count < parallel_tasks ? pr->count : parallel_tasks;

    for (int i = 0; i < tasks_to_execute; i++)
    {
        Msg *task = get_request(pr, i);
        if (task != NULL && task->occurrences == 0)
        {
            // Execute a tarefa em um processo separado
            pid_t pid = fork();
            if (pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0)
            {
                // Execute a tarefa no processo filho
                execute_task(task, output_folder);
                exit(EXIT_SUCCESS);
            }
            else
            {
                // Aguarde a conclusão do processo filho
                wait(NULL);
                task->occurrences++;
            }
        }
    }
}

void request_status(int pid) {
    char buffer[1024];

    int fd = open(ORCHESTRATOR, O_WRONLY);
    if (fd == -1) {
        perror("Open Error");
        exit(EXIT_FAILURE);
    }

    // Construindo a mensagem para solicitar o status
    Msg msg;
    strcpy(msg.program_and_args, "status");
    msg.pid = getpid();

    // Enviando a mensagem para o servidor
    ssize_t written_bytes = write(fd, &msg, sizeof(Msg));
    if (written_bytes != sizeof(Msg)) {
        perror("Write Error");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    // Abra o FIFO para receber a resposta do servidor
    snprintf(buffer, 1024, "tmp/FIFO_%d", getpid());
    int fd_ret = open(buffer, O_RDONLY);
    if (fd_ret == -1) {
        perror("Open Error");
        exit(EXIT_FAILURE);
    }
    // Ciclo para receber e imprimir mensagens do servidor
    char received_msg[1024];
    ssize_t read_bytes;
    while ((read_bytes = read(fd_ret, received_msg, sizeof(received_msg))) > 0)
    {
        // Adicionando o terminador de string
        received_msg[read_bytes] = '\0';
        write(STDOUT_FILENO, received_msg, read_bytes);
    }

    if (read_bytes == -1)
    {
        perror("Read Error");
        close(fd_ret);
        exit(EXIT_FAILURE);
    }

    close(fd_ret);
    unlink(buffer);
}

void process_status_request(PROCESS_REQUESTS *pr, const char *output_folder, int fd_ret)
{
    // Verificar se pr é NULL
    if (!pr)
    {
        write(fd_ret, "No tasks to display\n", strlen("No tasks to display\n"));
        return;
    }

    // Escrever o cabeçalho
    write(fd_ret, "Status of tasks:\n", strlen("Status of tasks:\n"));

    // Escrever as tarefas em execução
    write(fd_ret, "\nTasks in execution:\n", strlen("\nTasks in execution:\n"));
    for (int i = 0; i < pr->count; i++)
    {
        Msg *task = &pr->requests[i];
        if (task && task->status == RUNNING)
        {
            char buffer[50];
            sprintf(buffer, "Task %d: RUNNING\n", task->id);
            write(fd_ret, buffer, strlen(buffer));
        }
    }

    // Escrever as tarefas agendadas
    write(fd_ret, "\nScheduled tasks:\n", strlen("\nScheduled tasks:\n"));
    for (int i = 0; i < pr->count; i++)
    {
        Msg *task = &pr->requests[i];
        if (task && task->status == NEW)
        {
            char buffer[50];
            sprintf(buffer, "Task %d: NEW\n", task->id);
            write(fd_ret, buffer, strlen(buffer));
        }
    }

    // Escrever as tarefas concluídas
    write(fd_ret, "\nCompleted tasks:\n", strlen("\nCompleted tasks:\n"));
    for (int i = 0; i < pr->count; i++)
    {
        Msg *task = &pr->requests[i];
        if (task && task->status == DONE)
        {
            char buffer[100];
            sprintf(buffer, "Task %d: DONE in %.2f ms\n", task->id, task->execution_time);
            write(fd_ret, buffer, strlen(buffer));
        }
    }
}



void change_process_status(PROCESS_REQUESTS *pr, int id, TaskStatus new_status, double time)
{
    for (int i = 0; i < pr->count; i++)
    {
        if (pr->requests[i].id == id)
        {
            pr->requests[i].status = new_status;
            pr->requests[i].execution_time = time;
            return; // Uma vez que encontramos o processo com o ID especificado, podemos sair da função
        }
    }
    // Se chegarmos aqui, significa que o ID especificado não foi encontrado
    fprintf(stderr, "Process with ID %d not found\n", id);
}

int exec_command(char *arg)
{
    char *exec_args[10];

    char *string;
    int exec_ret = 0;
    int i = 0;

    char *command = strdup(arg);

    string = strtok(command, " ");

    while (string != NULL)
    {
        exec_args[i] = string;
        string = strtok(NULL, " ");
        i++;
    }

    exec_args[i] = NULL;

    exec_ret = execvp(exec_args[0], exec_args);

    return exec_ret;
}
