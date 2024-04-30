#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

//ainda por usar
/*
int mysystem(const char *command) {
    int res = -1;
    char* args[20];
    int i = 0;

	char* input = strdup(command);

    char* token = strtok(input, " ");
    while (token != NULL && i < 19) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;


    pid_t pid = fork();
	switch (pid)
	{
	case -1:
		res = -1;
		break;
	case 0:
		res = execvp(args[0], args);
        _exit(-1);
	default:
		int status;
        wait(&status);
		if (WEXITSTATUS(status) == 255) res = -1;
		else res = WEXITSTATUS(status);
		break;
	}


	free(input);
    return res;
}
*/