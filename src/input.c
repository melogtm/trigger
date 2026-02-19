#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"

char *trigger_read_line(void) {
    char *line = NULL;
    /*
     * Why buffer_size is equal to 0? Because getline will allocate a buffer for us if the line pointer
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

