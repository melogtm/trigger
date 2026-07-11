#include "input.h"
#include "utils/utils.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *trigger_read_line(void) {
    char *line = NULL;
    size_t buffer_size = 0;
    ssize_t result;

    do {
        result = getline(&line, &buffer_size, stdin);
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        }

        perror("readline");
        exit(EXIT_FAILURE);
    }

    return line;
}

char **trigger_split_line(const char *line) {
    TokenList *tl = parse_line_with_quotes(line);

    if (tl == NULL) {
        return NULL;
    }

    char **argv = tl->argv;
    free(tl->glob_eligible);
    free(tl);
    return argv;
}

char **trigger_split_line_ex(const char *line, int **out_glob_eligible) {
    TokenList *tl = parse_line_with_quotes(line);

    if (tl == NULL) {
        if (out_glob_eligible != NULL) {
            *out_glob_eligible = NULL;
        }
        return NULL;
    }

    if (out_glob_eligible != NULL) {
        *out_glob_eligible = tl->glob_eligible;
    } else {
        free(tl->glob_eligible);
    }

    char **argv = tl->argv;
    free(tl);
    return argv;
}
