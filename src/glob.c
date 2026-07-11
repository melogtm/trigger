#include "glob_expand.h"
#include "utils/utils.h"
#include <glob.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int has_metachars(const char *s) {
    return strchr(s, '*') != NULL || strchr(s, '?') != NULL || strchr(s, '[') != NULL;
}

static void grow_if_needed(char ***argv, int **eligible, size_t *capacity, size_t count) {
    if (count < *capacity) {
        return;
    }

    if (*capacity > SIZE_MAX / 2) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    size_t new_cap = *capacity * 2;

    *argv = xrealloc(*argv, (new_cap + 1) * sizeof(char *));
    *eligible = xrealloc(*eligible, (new_cap + 1) * sizeof(int));
    *capacity = new_cap;
}

void expand_globs(TokenList *tl) {
    if (tl == NULL || tl->argv == NULL || tl->glob_eligible == NULL) {
        return;
    }

    int has_any = 0;

    for (size_t i = 0; tl->argv[i] != NULL; i++) {
        if (tl->glob_eligible[i] && has_metachars(tl->argv[i])) {
            has_any = 1;
            break;
        }
    }

    if (!has_any) {
        return;
    }

    size_t capacity = 8;
    size_t count = 0;
    char **new_argv = xmalloc((capacity + 1) * sizeof(char *));
    int *new_eligible = xmalloc((capacity + 1) * sizeof(int));

    for (size_t i = 0; tl->argv[i] != NULL; i++) {
        if (tl->glob_eligible[i] && has_metachars(tl->argv[i])) {
            glob_t g;
            int r = glob(tl->argv[i], GLOB_NOCHECK, NULL, &g);

            if (r != 0) {
                globfree(&g);
                grow_if_needed(&new_argv, &new_eligible, &capacity, count);
                new_argv[count] = tl->argv[i];
                new_eligible[count] = 0;
                count++;
                continue;
            }

            for (size_t j = 0; j < g.gl_pathc; j++) {
                grow_if_needed(&new_argv, &new_eligible, &capacity, count);
                new_argv[count] = xstrdup(g.gl_pathv[j]);
                new_eligible[count] = 0;
                count++;
            }

            free(tl->argv[i]);
            globfree(&g);
        } else {
            grow_if_needed(&new_argv, &new_eligible, &capacity, count);
            new_argv[count] = tl->argv[i];
            new_eligible[count] = tl->glob_eligible[i];
            count++;
        }
    }

    new_argv[count] = NULL;

    free(tl->argv);
    free(tl->glob_eligible);

    tl->argv = new_argv;
    tl->glob_eligible = new_eligible;
    tl->count = count;
}
