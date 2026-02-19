#include <stdio.h>
#include <stdlib.h>
#include "input.h"
#include "execute.h"

#define true 1

void trigger_loop(void) {
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

int main(void) {
    trigger_loop();

    return EXIT_SUCCESS;
}
