#include <stdio.h>
#include <stdlib.h>
#include "input.h"
#include "execute.h"
#include "utils/utils.h"

void trigger_loop(void) {
    int status;

    do {
        printf("> ");
        char *line = trigger_read_line();
        char **args = trigger_split_line(line);

        if (args == NULL) {
            free(line);
            status = true;
            continue;
        }

        status = trigger_execute(args);

        free(line);
        free_array_of_strings(args);
    } while (status);
}

int main(void) {
    trigger_loop();

    return EXIT_SUCCESS;
}
