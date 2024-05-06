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

int pipeline(int id, char* arg, char* output_folder) {
    int status;
    // Array para armazenar os comandos individuais analisados da string do pipeline
    char* commands[REQUESTS];
    // Número de comandos no pipeline
    int number_of_commands = parse_pipeline(arg, commands);

    // Número de pipes necessários para a comunicação entre os comandos
    int number_of_pipes = number_of_commands - 1;
    // Array para armazenar os IDs dos processos de cada comando
    pid_t pids[number_of_commands];
    // Array para armazenar os descritores de arquivo para cada pipe
    int pipes[number_of_pipes][2];

    // Caminho do arquivo para o arquivo de saída do pipeline
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/output%d.txt", output_folder, id);
    
    // Abre o arquivo de saída
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    // Itera por cada comando no pipeline
    for (int i = 0; i < number_of_commands; i++) {
        // Cria pipes entre os comandos
        if (i < number_of_pipes) pipe(pipes[i]);
        
        // Forka um processo filho para cada comando
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("Erro ao criar processo filho");
            exit(EXIT_FAILURE);
        } else if (pids[i] == 0) {
            // Processo filho
            if (i == 0) {
                // Primeiro comando no pipeline
                close(pipes[i][0]);
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i][1]);
                // Executa o comando
                exec_command_pipe(commands[i]);
                _exit(EXIT_SUCCESS); // Termina o processo filho
            } else if (i == number_of_commands - 1) {
                // Último comando no pipeline
                close(pipes[i - 1][1]);
                dup2(pipes[i - 1][0], STDIN_FILENO);
                close(pipes[i - 1][0]);
                dup2(fd, 1);
                dup2(fd, 2);
                close(fd);
                // Executa o comando
                exec_command_pipe(commands[i]);
                _exit(EXIT_SUCCESS); // Termina o processo filho
            } else {
                // Comando intermediário no pipeline
                dup2(pipes[i - 1][0], STDIN_FILENO);
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i - 1][0]);
                close(pipes[i][1]);
                close(pipes[i][0]);
                // Executa o comando
                exec_command_pipe(commands[i]);
                _exit(EXIT_SUCCESS); // Termina o processo filho
            }
        } else {
            // Processo pai
            // Fecha extremidades do pipe desnecessárias
            if (i == 0) {
                close(pipes[i][1]);
            } else if (i == number_of_commands - 1) {
                close(pipes[i - 1][0]);
            } else {
                close(pipes[i - 1][0]);
                close(pipes[i][1]);
            }
        }
    }

    // Espera todos os processos filhos terminarem
    for (int i = 0; i < number_of_commands; i++) {
        waitpid(pids[i], &status, 0);
    }

    return 0;
}