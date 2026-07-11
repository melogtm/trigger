#include "pipeline.h"
#include "builtins.h"
#include "execute.h"
#include "utils/utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

PipelineStage *trigger_parse_pipeline(char **tokens, int *num_stages) {
    int stage_count = 1;
    int total_tokens = 0;

    for (int i = 0; tokens[i] != NULL; i++) {
        total_tokens++;
        if (strcmp(tokens[i], "|") == 0) {
            stage_count++;
        }
    }

    PipelineStage *stages = malloc(stage_count * sizeof(PipelineStage));

    if (stages == NULL) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    int si = 0;
    int stage_start = 0;

    while (si < stage_count) {
        stages[si].argv = NULL;
        stages[si].infile = NULL;
        stages[si].outfile = NULL;
        stages[si].append = 0;

        int argv_count = 0;
        char *infile_val = NULL;
        char *outfile_val = NULL;
        int outfile_append = 0;

        int j = stage_start;

        while (tokens[j] != NULL && strcmp(tokens[j], "|") != 0) {
            if (strcmp(tokens[j], "<") == 0) {
                if (tokens[j + 1] != NULL && strcmp(tokens[j + 1], "|") != 0 &&
                    strcmp(tokens[j + 1], "<") != 0 && strcmp(tokens[j + 1], ">") != 0 &&
                    strcmp(tokens[j + 1], ">>") != 0) {
                    infile_val = tokens[j + 1];
                    j++;
                } else {
                    fprintf(stderr, "trigger: syntax error: expected filename after '<'\n");
                }
            } else if (strcmp(tokens[j], ">") == 0) {
                if (tokens[j + 1] != NULL && strcmp(tokens[j + 1], "|") != 0 &&
                    strcmp(tokens[j + 1], "<") != 0 && strcmp(tokens[j + 1], ">") != 0 &&
                    strcmp(tokens[j + 1], ">>") != 0) {
                    outfile_val = tokens[j + 1];
                    outfile_append = 0;
                    j++;
                } else {
                    fprintf(stderr, "trigger: syntax error: expected filename after '>'\n");
                }
            } else if (strcmp(tokens[j], ">>") == 0) {
                if (tokens[j + 1] != NULL && strcmp(tokens[j + 1], "|") != 0 &&
                    strcmp(tokens[j + 1], "<") != 0 && strcmp(tokens[j + 1], ">") != 0 &&
                    strcmp(tokens[j + 1], ">>") != 0) {
                    outfile_val = tokens[j + 1];
                    outfile_append = 1;
                    j++;
                } else {
                    fprintf(stderr, "trigger: syntax error: expected filename after '>>'\n");
                }
            } else {
                argv_count++;
            }
            j++;
        }

        stages[si].argv = malloc((argv_count + 1) * sizeof(char *));

        if (stages[si].argv == NULL) {
            fprintf(stderr, "allocation error\n");
            exit(EXIT_FAILURE);
        }

        int ai = 0;

        j = stage_start;

        while (tokens[j] != NULL && strcmp(tokens[j], "|") != 0) {
            if (strcmp(tokens[j], "<") == 0) {
                if (tokens[j + 1] != NULL && strcmp(tokens[j + 1], "|") != 0 &&
                    strcmp(tokens[j + 1], "<") != 0 && strcmp(tokens[j + 1], ">") != 0 &&
                    strcmp(tokens[j + 1], ">>") != 0) {
                    j++;
                }
            } else if (strcmp(tokens[j], ">") == 0) {
                if (tokens[j + 1] != NULL && strcmp(tokens[j + 1], "|") != 0 &&
                    strcmp(tokens[j + 1], "<") != 0 && strcmp(tokens[j + 1], ">") != 0 &&
                    strcmp(tokens[j + 1], ">>") != 0) {
                    j++;
                }
            } else if (strcmp(tokens[j], ">>") == 0) {
                if (tokens[j + 1] != NULL && strcmp(tokens[j + 1], "|") != 0 &&
                    strcmp(tokens[j + 1], "<") != 0 && strcmp(tokens[j + 1], ">") != 0 &&
                    strcmp(tokens[j + 1], ">>") != 0) {
                    j++;
                }
            } else {
                stages[si].argv[ai++] = tokens[j];
                tokens[j] = NULL;
            }
            j++;
        }
        stages[si].argv[ai] = NULL;

        stages[si].infile = infile_val;
        stages[si].outfile = outfile_val;
        stages[si].append = outfile_append;

        if (infile_val != NULL) {
            for (int k = 0; k < total_tokens; k++) {
                if (tokens[k] == infile_val) {
                    tokens[k] = NULL;
                }
            }
        }

        if (outfile_val != NULL) {
            for (int k = 0; k < total_tokens; k++) {
                if (tokens[k] == outfile_val) {
                    tokens[k] = NULL;
                }
            }
        }

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

static void free_pipeline(PipelineStage *stages, int num_stages) {
    for (int i = 0; i < num_stages; i++) {
        free(stages[i].infile);
        free(stages[i].outfile);
        if (stages[i].argv != NULL) {
            free_array_of_strings(stages[i].argv);
        }
    }
    free(stages);
}

int trigger_execute_pipeline(PipelineStage *stages, int num_stages) {
    if (num_stages == 1 && stages[0].infile == NULL && stages[0].outfile == NULL) {
        char **args = stages[0].argv;

        if (args[0] != NULL) {
            for (int i = 0; i < trigger_num_builtins(); i++) {
                if (strcmp(args[0], builtin_str[i]) == 0) {
                    int r = (*builtin_func[i])(args);
                    free_pipeline(stages, num_stages);
                    return r;
                }
            }

            int r = trigger_launch(args);
            free_pipeline(stages, num_stages);
            return r;
        }

        free_pipeline(stages, num_stages);
        return true;
    }

    if (num_stages == 1 && (stages[0].infile != NULL || stages[0].outfile != NULL)) {
        char **args = stages[0].argv;

        if (args[0] != NULL) {
            for (int i = 0; i < trigger_num_builtins(); i++) {
                if (strcmp(args[0], builtin_str[i]) == 0) {
                    int saved_stdin = dup(STDIN_FILENO);
                    int saved_stdout = dup(STDOUT_FILENO);

                    if (stages[0].infile != NULL) {
                        int fd = open(stages[0].infile, O_RDONLY);

                        if (fd < 0) {
                            perror("trigger");
                        } else {
                            dup2(fd, STDIN_FILENO);
                            close(fd);
                        }
                    }

                    if (stages[0].outfile != NULL) {
                        int flags = O_WRONLY | O_CREAT;

                        flags |= stages[0].append ? O_APPEND : O_TRUNC;
                        int fd = open(stages[0].outfile, flags, 0644);

                        if (fd < 0) {
                            perror("trigger");
                        } else {
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        }
                    }

                    int r = (*builtin_func[i])(args);

                    dup2(saved_stdin, STDIN_FILENO);
                    close(saved_stdin);
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(saved_stdout);

                    free_pipeline(stages, num_stages);
                    return r;
                }
            }
        }
    }

    int pipe_count = num_stages - 1;
    int *pipe_fds = NULL;

    if (pipe_count > 0) {
        pipe_fds = malloc((unsigned long) (2 * pipe_count) * sizeof(int));

        if (pipe_fds == NULL) {
            fprintf(stderr, "allocation error\n");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < pipe_count; i++) {
            if (pipe(pipe_fds + (2 * i)) < 0) {
                perror("trigger");

                for (int j = 0; j < i; j++) {
                    close(pipe_fds[2 * j]);
                    close(pipe_fds[(2 * j) + 1]);
                }

                free(pipe_fds);
                free_pipeline(stages, num_stages);
                return true;
            }
        }
    }

    pid_t *pids = malloc(num_stages * sizeof(pid_t));

    if (pids == NULL) {
        fprintf(stderr, "allocation error\n");
        free(pipe_fds);
        free_pipeline(stages, num_stages);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_stages; i++) {
        pids[i] = -1;
    }

    for (int i = 0; i < num_stages; i++) {
        pid_t pid = fork();

        if (pid == CHILD_PROCESS_EXITED) {
            if (i > 0) {
                dup2(pipe_fds[2 * (i - 1)], STDIN_FILENO);
            } else if (stages[i].infile != NULL) {
                int fd = open(stages[i].infile, O_RDONLY);

                if (fd < 0) {
                    perror("trigger");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (i < num_stages - 1) {
                dup2(pipe_fds[(2 * i) + 1], STDOUT_FILENO);
            } else if (stages[i].outfile != NULL) {
                int flags = O_WRONLY | O_CREAT;

                flags |= stages[i].append ? O_APPEND : O_TRUNC;
                int fd = open(stages[i].outfile, flags, 0644);

                if (fd < 0) {
                    perror("trigger");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            for (int j = 0; j < 2 * pipe_count; j++) {
                close(pipe_fds[j]);
            }

            char **args = stages[i].argv;

            if (args[0] != NULL) {
                for (int j = 0; j < trigger_num_builtins(); j++) {
                    if (strcmp(args[0], builtin_str[j]) == 0) {
                        (*builtin_func[j])(args);
                        exit(EXIT_SUCCESS);
                    }
                }

                if (execvp(args[0], args) < 0) {
                    perror("trigger");
                }
            }
            exit(EXIT_FAILURE);
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

    for (int i = 0; i < num_stages; i++) {
        if (pids[i] == -1) {
            continue;
        }
        do {
            waitpid(pids[i], &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    free(pids);
    free_pipeline(stages, num_stages);
    return true;
}
