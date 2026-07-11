#include "execute.h"
#include "builtins.h"
#include "glob_expand.h"
#include "pipeline.h"
#include "utils/utils.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int trigger_launch(char **args) {
    int status = 0;

    const pid_t pid = fork();

    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        if (execvp(args[0], args) == -1) {
            perror("trigger");
        }
        _exit(127);
    }

    if (pid < 0) {
        perror("trigger");
        return 1;
    }

    int w;
    do {
        w = waitpid(pid, &status, 0);
    } while (w == -1 && errno == EINTR);

    if (w == -1) {
        perror("trigger");
        return 1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }

    return 1;
}

ExecuteResult trigger_execute(TokenList *tl) {
    ExecuteResult result = {0, false};

    if (tl == NULL || tl->argv == NULL || tl->argv[0] == NULL) {
        result.status = 0;
        return result;
    }

    expand_globs(tl);

    int has_operator = 0;

    for (size_t i = 0; tl->argv[i] != NULL; i++) {
        if (tl->glob_eligible == NULL || tl->glob_eligible[i]) {
            if (classify_operator(tl->argv[i]) != OP_NONE) {
                has_operator = 1;
                break;
            }
        }
    }

    if (has_operator) {
        int num_stages = 0;
        PipelineStage *stages = trigger_parse_pipeline(tl->argv, tl->glob_eligible, &num_stages);

        if (stages == NULL) {
            result.status = 1;
            return result;
        }

        PipelineResult pr = trigger_execute_pipeline(stages, num_stages);
        result.status = pr.last_status;
        return result;
    }

    const Builtin *b = find_builtin(tl->argv[0]);

    if (b != NULL) {
        if (b->fn == trigger_exit) {
            result.should_exit = true;
        }
        result.status = b->fn(tl->argv);
        return result;
    }

    result.status = trigger_launch(tl->argv);
    return result;
}
