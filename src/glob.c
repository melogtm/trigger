#include "glob_expand.h"
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int has_metachars(const char *s) {
    return strchr(s, '*') != NULL || strchr(s, '?') != NULL || strchr(s, '[') != NULL;
}

static void grow_if_needed(char ***argv, int **eligible, int *capacity, int count) {
    if (count < *capacity) {
        return;
    }

    int new_cap = *capacity * 2;

    char **tmp_a = realloc(*argv, (new_cap + 1) * sizeof(char *));
    int *tmp_e = realloc(*eligible, (new_cap + 1) * sizeof(int));

    if (tmp_a == NULL || tmp_e == NULL) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    *argv = tmp_a;
    *eligible = tmp_e;
    *capacity = new_cap;
}

char **expand_globs(char **argv, int **glob_eligible_ptr) {
    if (glob_eligible_ptr == NULL) {
        return argv;
    }

    int *glob_eligible = *glob_eligible_ptr;

    if (glob_eligible == NULL) {
        return argv;
    }

    int has_any = 0;

    for (int i = 0; argv[i] != NULL; i++) {
        if (glob_eligible[i] && has_metachars(argv[i])) {
            has_any = 1;
            break;
        }
    }

    if (!has_any) {
        return argv;
    }

    int capacity = 8;
    int count = 0;
    char **new_argv = malloc((capacity + 1) * sizeof(char *));

    if (new_argv == NULL) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    int *new_eligible = malloc((capacity + 1) * sizeof(int));

    if (new_eligible == NULL) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; argv[i] != NULL; i++) {
        if (glob_eligible[i] && has_metachars(argv[i])) {
            glob_t g;
            int r = glob(argv[i], GLOB_NOCHECK, NULL, &g);

            if (r == GLOB_NOSPACE) {
                fprintf(stderr, "trigger: glob out of memory\n");
                globfree(&g);
                grow_if_needed(&new_argv, &new_eligible, &capacity, count);
                new_argv[count] = argv[i];
                new_eligible[count] = 0;
                count++;
                continue;
            }

            for (size_t j = 0; j < g.gl_pathc; j++) {
                grow_if_needed(&new_argv, &new_eligible, &capacity, count);
                new_argv[count] = strdup(g.gl_pathv[j]);
                new_eligible[count] = 0;
                count++;
            }

            free(argv[i]);
            globfree(&g);
        } else {
            grow_if_needed(&new_argv, &new_eligible, &capacity, count);
            new_argv[count] = argv[i];
            new_eligible[count] = glob_eligible[i];
            count++;
        }
    }

    new_argv[count] = NULL;

    free(argv);
    free(glob_eligible);

    *glob_eligible_ptr = new_eligible;
    return new_argv;
}
