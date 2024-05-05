#ifndef DEFS_H
#define DEFS_H

#define ORCHESTRATOR "tmp/fifo_orchestrator"
#define CLIENT "fifo_client"
#define REQUESTS 100

typedef enum {
    false,
    true
} bool;

typedef enum{
    NEW,
    RUNNING,
    DONE
} TaskStatus;



typedef struct msg{
    int id;
    int pid;
    int time_estimated;
    int execution_time;
    int occurrences;
    char program_and_args[300];
    TaskStatus status;
} Msg;

typedef struct process_requests {
    Msg requests[REQUESTS];
    int count;
} PROCESS_REQUESTS;

void make_fifo(const char *fifo_name);
void open_fifo(int *fd, const char *fifo_name, int flags);
void send_client_response(int pid, int id);
PROCESS_REQUESTS* init_process_requests();
void free_process_requests(PROCESS_REQUESTS *pr);
bool add_request(PROCESS_REQUESTS *pr, Msg msg);
Msg* get_request(PROCESS_REQUESTS *pr, int index);
void remove_request(PROCESS_REQUESTS *pr, int index);
void handle_request(PROCESS_REQUESTS *pr, Msg *msg);
void execute_task(Msg *msg, const char *output_folder);


#endif