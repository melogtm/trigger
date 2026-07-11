#include "pipeline.h"
#include "builtins.h"
#include "execute.h"
#include "utils/utils.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARGV_INIT_CAP 8

Operator classify_operator(const char *token) {
    if (token == NULL) {
        return OP_NONE;
    }
    if (strcmp(token, "|") == 0) {
        return OP_PIPE;
    }
    if (strcmp(token, "<") == 0) {
        return OP_REDIR_IN;
    }
    if (strcmp(token, ">") == 0) {
        return OP_REDIR_OUT;
    }
    if (strcmp(token, ">>") == 0) {
        return OP_REDIR_APPEND;
    }
    return OP_NONE;
}

static int is_operator_token(const char *token, const int *glob_eligible, int idx) {
    if (token == NULL) {
        return 0;
    }
    if (glob_eligible != NULL && !glob_eligible[idx]) {
        return 0;
    }
    return classify_operator(token) != OP_NONE;
}

static void argv_grow(char ***argv, size_t *cap, size_t count) {
    if (count < *cap) {
        return;
    }
    if (*cap > SIZE_MAX / 2) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }
    *cap *= 2;
    *argv = xrealloc(*argv, (*cap + 1) * sizeof(char *));
}

PipelineStage *trigger_parse_pipeline(char **tokens, const int *glob_eligible, int *num_stages) {
    int stage_count = 1;
    int total_tokens = 0;
    int error = 0;

    for (int i = 0; tokens[i] != NULL; i++) {
        total_tokens++;
        if (is_operator_token(tokens[i], glob_eligible, i) &&
            classify_operator(tokens[i]) == OP_PIPE) {
            stage_count++;
        }
    }

    PipelineStage *stages = xmalloc(stage_count * sizeof(PipelineStage));

    int si = 0;
    int stage_start = 0;

    while (si < stage_count) {
        stages[si].argv = NULL;
        stages[si].infile = NULL;
        stages[si].outfile = NULL;
        stages[si].append = 0;

        char **argv = xmalloc((ARGV_INIT_CAP + 1) * sizeof(char *));
        size_t argv_cap = ARGV_INIT_CAP;
        size_t ai = 0;

        char *infile_val = NULL;
        char *outfile_val = NULL;
        int outfile_append = 0;

        int j = stage_start;

        while (tokens[j] != NULL &&
               !(is_operator_token(tokens[j], glob_eligible, j) &&
                 classify_operator(tokens[j]) == OP_PIPE)) {

            if (!is_operator_token(tokens[j], glob_eligible, j)) {
                argv_grow(&argv, &argv_cap, ai);
                argv[ai++] = tokens[j];
                tokens[j] = NULL;
                j++;
                continue;
            }

            Operator op = classify_operator(tokens[j]);

            if (op == OP_REDIR_IN || op == OP_REDIR_OUT || op == OP_REDIR_APPEND) {
                if (tokens[j + 1] == NULL ||
                    is_operator_token(tokens[j + 1], glob_eligible, j + 1)) {
                    fprintf(stderr, "trigger: syntax error near '%s'\n", tokens[j]);
                    error = 1;
                    break;
                }

                if (op == OP_REDIR_IN) {
                    infile_val = tokens[j + 1];
                } else {
                    outfile_val = tokens[j + 1];
                    outfile_append = (op == OP_REDIR_APPEND);
                }
                j += 2;
            } else {
                j++;
            }
        }

        if (error) {
            free(argv);
            int k;
            for (k = 0; k < si; k++) {
                free(stages[k].argv);
            }
            free(stages);
            for (int t = 0; t < total_tokens; t++) {
                free(tokens[t]);
            }
            *num_stages = 0;
            return NULL;
        }

        argv[ai] = NULL;
        stages[si].argv = argv;
        stages[si].infile = infile_val;
        stages[si].outfile = outfile_val;
        stages[si].append = outfile_append;

        if (tokens[j] != NULL) {
            j++;
        }
        stage_start = j;
        si++;
    }

    for (int i = 0; i < total_tokens; i++) {
        free(tokens[i]);
    }

    *num_stages = stage_count;
    return stages;
}

void trigger_free_pipeline(PipelineStage *stages, int num_stages) {
    for (int i = 0; i < num_stages; i++) {
        free(stages[i].infile);
        free(stages[i].outfile);
        if (stages[i].argv != NULL) {
            free_array_of_strings(stages[i].argv);
        }
    }
    free(stages);
}

static int apply_redirection(int infile_override, const char *infile, const char *outfile,
                             int append) {
    if (infile_override >= 0 && infile != NULL) {
        int fd = open(infile, O_RDONLY);
        if (fd < 0) {
            perror("trigger");
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("trigger");
            close(fd);
            return -1;
        }
        close(fd);
    }

    if (outfile != NULL) {
        int flags = O_WRONLY | O_CREAT;
        flags |= append ? O_APPEND : O_TRUNC;
        int fd = open(outfile, flags, FILE_MODE);
        if (fd < 0) {
            perror("trigger");
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("trigger");
            close(fd);
            return -1;
        }
        close(fd);
    }

    return 0;
}

PipelineResult trigger_execute_pipeline(PipelineStage *stages, int num_stages) {
    PipelineResult result = {0};

    if (num_stages == 0 || stages == NULL) {
        result.last_status = 1;
        return result;
    }

    if (num_stages == 1 && stages[0].infile == NULL && stages[0].outfile == NULL) {
        char **args = stages[0].argv;

        if (args[0] != NULL) {
            const Builtin *b = find_builtin(args[0]);
            if (b != NULL) {
                result.last_status = b->fn(args);
                trigger_free_pipeline(stages, num_stages);
                return result;
            }

            result.last_status = trigger_launch(args);
            trigger_free_pipeline(stages, num_stages);
            return result;
        }

        trigger_free_pipeline(stages, num_stages);
        return result;
    }

    if (num_stages == 1 && (stages[0].infile != NULL || stages[0].outfile != NULL)) {
        char **args = stages[0].argv;

        if (args[0] != NULL) {
            const Builtin *b = find_builtin(args[0]);
            if (b != NULL) {
                int saved_stdin = dup(STDIN_FILENO);
                int saved_stdout = dup(STDOUT_FILENO);

                if (saved_stdin == -1 || saved_stdout == -1) {
                    perror("trigger");
                    if (saved_stdin != -1) {
                        close(saved_stdin);
                    }
                    trigger_free_pipeline(stages, num_stages);
                    result.last_status = 1;
                    return result;
                }

                int redir_error =
                    apply_redirection(1, stages[0].infile, stages[0].outfile, stages[0].append);

                if (redir_error == 0) {
                    result.last_status = b->fn(args);
                } else {
                    result.last_status = 1;
                }

                if (dup2(saved_stdin, STDIN_FILENO) == -1) {
                    perror("trigger");
                }
                close(saved_stdin);
                if (dup2(saved_stdout, STDOUT_FILENO) == -1) {
                    perror("trigger");
                }
                close(saved_stdout);

                trigger_free_pipeline(stages, num_stages);
                return result;
            }
        }
    }

    int pipe_count = num_stages - 1;
    int *pipe_fds = NULL;

    if (pipe_count > 0) {
        pipe_fds = xmalloc((unsigned long)(2 * pipe_count) * sizeof(int));

        for (int i = 0; i < pipe_count; i++) {
            if (pipe(pipe_fds + (2 * i)) < 0) {
                perror("trigger");

                for (int j = 0; j < i; j++) {
                    close(pipe_fds[2 * j]);
                    close(pipe_fds[(2 * j) + 1]);
                }

                free(pipe_fds);
                trigger_free_pipeline(stages, num_stages);
                result.last_status = 1;
                return result;
            }
        }
    }

    pid_t *pids = xmalloc(num_stages * sizeof(pid_t));

    for (int i = 0; i < num_stages; i++) {
        pids[i] = -1;
    }

    for (int i = 0; i < num_stages; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);

            if (i > 0) {
                if (dup2(pipe_fds[2 * (i - 1)], STDIN_FILENO) == -1) {
                    perror("trigger");
                    _exit(EXIT_FAILURE);
                }
            } else {
                int redir_err = apply_redirection(1, stages[i].infile, NULL, 0);
                if (redir_err != 0) {
                    _exit(EXIT_FAILURE);
                }
            }

            if (i < num_stages - 1) {
                if (dup2(pipe_fds[(2 * i) + 1], STDOUT_FILENO) == -1) {
                    perror("trigger");
                    _exit(EXIT_FAILURE);
                }
            } else {
                int redir_err = apply_redirection(0, NULL, stages[i].outfile,
                                                  stages[i].append);
                if (redir_err != 0) {
                    _exit(EXIT_FAILURE);
                }
            }

            for (int j = 0; j < 2 * pipe_count; j++) {
                close(pipe_fds[j]);
            }

            char **args = stages[i].argv;

            if (args[0] != NULL) {
                const Builtin *b = find_builtin(args[0]);
                if (b != NULL) {
                    int r = b->fn(args);
                    _exit(r);
                }

                if (execvp(args[0], args) < 0) {
                    perror("trigger");
                }
            }
            _exit(127);
        } else if (pid < 0) {
            perror("trigger");
        } else {
            pids[i] = pid;
        }
    }

    for (int j = 0; j < 2 * pipe_count; j++) {
        close(pipe_fds[j]);
    }
    free(pipe_fds);

    int status;
    int last_exit_status = 0;

    for (int i = 0; i < num_stages; i++) {
        if (pids[i] == -1) {
            continue;
        }
        int w;
        do {
            w = waitpid(pids[i], &status, 0);
        } while (w == -1 && errno == EINTR);

        if (w == -1) {
            perror("trigger");
            continue;
        }

        if (i == num_stages - 1) {
            if (WIFEXITED(status)) {
                last_exit_status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                last_exit_status = 128 + WTERMSIG(status);
            }
        }
    }

    free(pids);
    trigger_free_pipeline(stages, num_stages);
    result.last_status = last_exit_status;
    return result;
}
