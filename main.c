#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define true 1
#define false 0

#define TRIGGER_TOK_BUFFER_SIZE 64
#define TRIGGER_TOK_DELIM " \t\r\n\a"

char * trigger_read_line(void) {
    char * line = NULL;
    /*
     * Why buffer_isze is equal to 0? Because getline will allocate a buffer for us if the line pointer
     * is NULL and the buffer size is 0.
     *
     * If the buffer size is not 0, getline will try to read into the buffer and if the buffer is not large enough,
     * it will return an error.
     *
     * Thank God humans invented getline.
     */
    size_t buffer_size = 0;

    if (getline(&line, &buffer_size, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        }

        perror("readline");
        exit(EXIT_FAILURE);
    }

    return line;
};

char ** trigger_split_line(char * line) {
    int buffer_size = TRIGGER_TOK_BUFFER_SIZE, position = 0;
    char ** tokens = malloc(buffer_size * sizeof(char *));
    char *token = strtok(line, TRIGGER_TOK_DELIM);

    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }


    while (token != NULL) {
        tokens[position++] = token;

        if (position >= buffer_size) {
            buffer_size += TRIGGER_TOK_BUFFER_SIZE;

            char ** new_tokens = realloc(tokens, buffer_size * sizeof(char *));

            if (new_tokens) {
                tokens = new_tokens;
            } else {
                fprintf(stderr, "allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TRIGGER_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int trigger_launch(char ** args) {
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

// List of builtin commands and their corresponding functions.
int trigger_cd(char **args);
int trigger_help(char **args);
int trigger_exit(char **args);


// List of builtin commands
char *builtin_str[] = {"cd", "help", "exit"};

/*
 * builtin_func is an array of pointers to functions that take a char** as an argument and return an int.
 * This is the devil.
 */
int (*builtin_func[]) (char **) = {&trigger_cd, &trigger_help, &trigger_exit};



int trigger_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int trigger_cd(char ** args) {
    if (args[1] == NULL) {
        fprintf(stderr, "trigger: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("trigger");
        }
    }

    return true;
}

int trigger_help(char ** args) {
    printf("Trigger: A simple shell written in C\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < trigger_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return true;
}

int trigger_exit(char ** args) {
    return EXIT_SUCCESS;
}

int trigger_execute(char ** args) {
    if (args[0] == NULL) {
        // An empty command was entered.
        return true;
    }

    for (int i = 0; i < trigger_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return trigger_launch(args);
}


void trigger_loop() {
    int status;

    do {
        printf("> ");
        char *line = trigger_read_line();
        char **args = trigger_split_line(line);
        status = trigger_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main() {
    trigger_loop();

    return EXIT_SUCCESS;
}
