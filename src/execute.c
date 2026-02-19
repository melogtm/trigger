#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "execute.h"
#include "builtins.h"

#define true 1

int trigger_launch(char **args) {
    int status;

    const pid_t pid = fork();

    if (pid == CHILD_PROCESS_EXITED) {
        if (execvp(args[0], args) == EXEC_RETURNED_FAILURE) {
            perror("trigger");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("trigger");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return true;
}

int trigger_execute(char **args) {
    if (args[0] == NULL) {
        return true;
    }

    for (int i = 0; i < trigger_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return trigger_launch(args);
}

