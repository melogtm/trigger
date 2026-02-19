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

    if (pid == 0) {
        // It is a child process
        if (execvp(args[0], args) == -1) {
            // A process cannot ever return from execvp if it is successful.
            perror("trigger");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("trigger");
    } else {
        // It is a parent process
        do {
            waitpid(pid, &status, WUNTRACED);

            // the macros WIF EXITED and WIF SIGNALED are used to check if the child process has exited or
            // was terminated by a signal.
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

