#include "execute.h"
#include "input.h"
#include "nepeta.h"
#include "utils/utils.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void trigger_loop(void) {
    int last_status = 0;

    for (;;) {
        printf("%s", trigger_nepeta_prompt());
        fflush(stdout);
        char *line = trigger_read_line();
        TokenList *tl = parse_line_with_quotes(line);

        if (tl == NULL) {
            free(line);
            continue;
        }

        ExecuteResult r = trigger_execute(tl);
        last_status = r.status;

        free(line);
        token_list_free(tl);

        if (r.should_exit) {
            break;
        }
    }

    exit(last_status);
}

int main(void) {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    trigger_nepeta_init_from_env();
    trigger_nepeta_print_banner();

    trigger_loop();

    return EXIT_SUCCESS;
}
