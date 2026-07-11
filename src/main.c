#include <stdio.h>
#include <stdlib.h>
#include "input.h"
#include "execute.h"
#include "utils/utils.h"

void trigger_loop(void) {
    int status;

    do {
        printf("> ");
        fflush(stdout);
        char *line = trigger_read_line();
        int *glob_eligible = NULL;
        char **args = trigger_split_line_ex(line, &glob_eligible);

        if (args == NULL) {
            free(line);
            free(glob_eligible);
            status = true;
            continue;
        }

        status = trigger_execute(&args, &glob_eligible);

        free(line);
        free(glob_eligible);
        free_array_of_strings(args);
    } while (status);
}

int main(void) {
    trigger_loop();

    return EXIT_SUCCESS;
}
