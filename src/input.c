#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"

char *trigger_read_line(void) {
    char *line = NULL;
    size_t buffer_size = TRIGGER_AUTOMATIC_BUFFER_SIZE;

    if (getline(&line, &buffer_size, stdin) == GETLINE_REACHED_END_OF_FILE_OR_ERROR) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        }

        perror("readline");
        exit(EXIT_FAILURE);
    }

    return line;
}

char **trigger_split_line(char *line) {
    int buffer_size = TRIGGER_TOK_BUFFER_SIZE, position = 0;
    char **tokens = malloc(buffer_size * sizeof(char *));
    char *token = strtok(line, TRIGGER_TOK_DELIM);

    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (token != NULL) {
        tokens[position++] = token;

        if (position >= buffer_size) {
            buffer_size += TRIGGER_TOK_BUFFER_SIZE;

            char **new_tokens = realloc(tokens, buffer_size * sizeof(char *));

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

