#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "execute.h"
#include "builtins.h"
#include "glob_expand.h"
#include "pipeline.h"
#include "utils/utils.h"

int trigger_launch(char **args) {
    int status;

    const pid_t pid = fork();

    if (pid == CHILD_PROCESS_EXITED) {
        if (execvp(args[0], args) == EXEC_RETURNED_FAILURE) {
            perror("trigger");
        }
        exit(EXIT_FAILURE);
    }

    if (pid < 0) {
        perror("trigger");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return true;
}

int trigger_execute(char ***args_ptr, int **glob_eligible_ptr) {
    char **args = *args_ptr;
    int *glob_eligible = (glob_eligible_ptr != NULL) ? *glob_eligible_ptr : NULL;

    if (args == NULL || args[0] == NULL) {
        return true;
    }

    args = expand_globs(args, glob_eligible_ptr);
    *args_ptr = args;
    glob_eligible = (glob_eligible_ptr != NULL) ? *glob_eligible_ptr : NULL;

    int has_operator = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (glob_eligible == NULL || glob_eligible[i]) {
            if (strcmp(args[i], "|") == 0 || strcmp(args[i], "<") == 0
                || strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
                has_operator = 1;
                break;
            }
        }
    }

    if (has_operator) {
        int num_stages = 0;
        PipelineStage *stages = trigger_parse_pipeline(args, &num_stages);
        int result = trigger_execute_pipeline(stages, num_stages);

        free(args);
        *args_ptr = NULL;
        return result;
    }

    for (int i = 0; i < trigger_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return trigger_launch(args);
}
