#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "/home/joao/SO/grupo-18/include/defs.h"


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

    int fildes = open("FIFO", O_RDWR);
    if (fildes == -1)
    {
        perror("open error");
        exit(1);
    }

    while (1)
    {
        Msg new;
        ssize_t read_bytes;

        // Leitura do FIFO
        read_bytes = read(fildes, &new, sizeof(Msg));
        if (read_bytes <= 0)
        {
            // Verifica se a leitura foi bem-sucedida
            if (read_bytes == 0)
            {
                // FIFO foi fechado pelo outro lado
                printf("FIFO closed\n");
            }
            else
            {
                // Erro na leitura
                perror("read error");
            }
            break;
        }

        // Processamento da mensagem
        int occurrences = count_needle(new.program_and_args, "needle");
        new.occurrences = occurrences;

        pid_t pid = fork();
        if (pid == -1)
        {
            // Verifica se o fork foi bem-sucedido
            perror("fork error");
            break;
        }
        else if (pid == 0)
        {
            // Processo filho

            // Criação do arquivo de saída
            char output[1024];
            int fd_output = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_output == -1)
            {
                // Verifica se a abertura do arquivo foi bem-sucedida
                perror("open output error");
                exit(1);
            }

            // Redirecionamento da saída padrão e de erro
            dup2(fd_output, STDOUT_FILENO);
            dup2(fd_output, STDERR_FILENO);
            close(fd_output);

            // Execução do comando shell
            execlp("sh", "sh", "-c", new.program_and_args, NULL);
            perror("execlp error");
            _exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            // Processo pai
            int status;
            waitpid(pid, &status, 0);

            // Verifica se o processo filho terminou com sucesso
            if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
            {
                printf("Processo filho %d terminou com sucesso\n", pid);
            }
            else
            {
                printf("Processo filho %d terminou com erro\n", pid);
            }
        }
    }

    // Fecho do FIFO
    close(fildes);
    unlink("FIFO");

    return 0;
}