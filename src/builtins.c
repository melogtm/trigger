#include "builtins.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const Builtin builtins[] = {
    {"cd", trigger_cd},       {"help", trigger_help}, {"exit", trigger_exit},
    {"pwd", trigger_pwd},     {"echo", trigger_echo}, {"export", trigger_export},
    {"unset", trigger_unset},
};

static const int builtin_count = sizeof(builtins) / sizeof(builtins[0]);

int trigger_num_builtins(void) { return builtin_count; }

const Builtin *find_builtin(const char *name) {
    for (int i = 0; i < builtin_count; i++) {
        if (strcmp(builtins[i].name, name) == 0) {
            return &builtins[i];
        }
    }
    return NULL;
}

int trigger_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "trigger: expected argument to \"cd\"\n");
        return 1;
    }
    if (chdir(args[1]) != 0) {
        perror("trigger");
        return 1;
    }

    return 0;
}

int trigger_help(char **args __attribute__((unused))) {
    printf("Trigger: A simple shell written in C\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < builtin_count; i++) {
        printf("  %s\n", builtins[i].name);
    }

    printf("Use the man command for information on other programs.\n");
    return 0;
}

int trigger_exit(char **args) {
    if (args[1] != NULL) {
        char *endptr;
        long n = strtol(args[1], &endptr, 10);
        if (*endptr == '\0' && n >= 0 && n <= 255) {
            return (int) n;
        }
        fprintf(stderr, "trigger: exit: %s: numeric argument required\n", args[1]);
        return 2;
    }
    return EXIT_SUCCESS;
}

int trigger_pwd(char **args __attribute__((unused))) {
    char *cwd = getcwd(NULL, 0);

    if (cwd == NULL) {
        perror("trigger");
        return 1;
    }

    printf("%s\n", cwd);
    free(cwd);
    return 0;
}

int trigger_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        if (i > 1) {
            printf(" ");
        }
        printf("%s", args[i]);
    }
    printf("\n");
    return 0;
}

int trigger_export(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        char *equals = strchr(args[i], '=');

        if (equals == NULL) {
            continue;
        }

        char *name = strndup(args[i], equals - args[i]);

        if (name == NULL) {
            perror("trigger");
            continue;
        }

        setenv(name, equals + 1, 1);
        free(name);
    }
    return 0;
}

int trigger_unset(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        unsetenv(args[i]);
    }
    return 0;
}
