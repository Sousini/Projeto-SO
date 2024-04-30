#define SERVER "fifo_server"
#define CLIENT "fifo_client"

typedef struct msg{
    int pid;
    int time_estimated;
    int occurrences;
    char program_and_args[300];
} Msg;