#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "utils/utils.h"

char *trigger_read_line(void) {
    char *line = NULL;
    size_t buffer_size = TRIGGER_AUTOMATIC_BUFFER_SIZE;

    if (getline(&line, &buffer_size, stdin) == GET_LINE_REACHED_END_OF_FILE_OR_ERROR) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        }

        perror("readline");
        exit(EXIT_FAILURE);
    }

    return line;
}

char **trigger_split_line(const char *line) {
    char** tokens = parse_line_with_quotes(line);

    if (tokens == NULL) {
        return NULL;
    }

    return tokens;
}

