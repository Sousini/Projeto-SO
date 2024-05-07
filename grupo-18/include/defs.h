#ifndef DEFS_H
#define DEFS_H

#define ORCHESTRATOR "tmp/fifo_orchestrator"
#define CLIENT "fifo_client"
#define REQUESTS 100

typedef enum
{
    false,
    true
} bool;

typedef enum
{
    NEW,
    RUNNING,
    SCHEDULED,
    DONE,
    WAIT
} TaskStatus;

typedef struct msg
{
    int id;
    int pid;
    int time_estimated;
    double execution_time;
    int occurrences;
    char program_and_args[300];
    TaskStatus status;
} Msg;

typedef struct process_requests
{
    Msg requests[REQUESTS];
    int count;
} PROCESS_REQUESTS;

void make_fifo(const char *fifo_name);
void open_fifo(int *fd, const char *fifo_name, int flags);
PROCESS_REQUESTS *init_process_requests();
void free_process_requests(PROCESS_REQUESTS *pr);
bool add_request(PROCESS_REQUESTS *pr, Msg msg);
void execute_task(Msg *msg, const char *output_folder);
void request_status(int pid);
void process_status_request(PROCESS_REQUESTS *pr, const char *output_folder, int fd_ret);
void change_process_status(PROCESS_REQUESTS *pr, int id, TaskStatus new_status, double time);
int exec_command(char *arg);

#endif