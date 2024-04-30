#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "/home/joao/SO/grupo-18/include/defs.h"

int main(int argc, char *argv[])
{
    // Extrair o tempo estimado
    int time_estimated = atoi(argv[1]);

    // Extrair o nome do programa e seus argumentos
    char *program_and_args = argv[4];

    // Criar a mensagem para enviar ao servidor
    Msg msg;
    msg.time_estimated = time_estimated;
    strncpy(msg.program_and_args, program_and_args, sizeof(msg.program_and_args));
    msg.program_and_args[sizeof(msg.program_and_args) - 1] = '\0';


    // criar o FIFO do client
    if (mkfifo("FIFO", 0666) == -1)
    {
        perror("create error");
    }

    // Abrir o FIFO para escrita
    int fd = open("FIFO", O_WRONLY);
    if (fd == -1)
    {
        perror("open error");
    }

    // Enviar a mensagem ao servidor
    if (write(fd, &msg, sizeof(Msg)) == -1) {
        perror("write error");
        close(fd);
        return 1;
    }
    close(fd);

    // Abrir o FIFO para leitura
    fd = open("FIFO", O_RDONLY);
    if (fd == -1)
    {
        perror("open error");
    }

    // Ler a msg de resposta
    if (read(fd, &msg, sizeof(Msg)) == -1) {
        perror("read error");
        close(fd);
        return 1;
    }
    close(fd);

    return 0;
}