#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtins.h"
#include "utils/utils.h"

char *builtin_str[] = {"cd", "help", "exit", "pwd", "echo", "export", "unset"};

int (*builtin_func[])(char **) = {&trigger_cd, &trigger_help, &trigger_exit, &trigger_pwd, &trigger_echo, &trigger_export, &trigger_unset};

int trigger_num_builtins(void) {
    return sizeof(builtin_str) / sizeof(char *);
}

int trigger_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "trigger: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != EXIT_SUCCESS) {
            perror("trigger");
        }
    }

    return true;
}

int trigger_help(char **args) {
    printf("Trigger: A simple shell written in C\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < trigger_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return true;
}

int trigger_exit(char **args) {
    return EXIT_SUCCESS;
}

int trigger_pwd(char **args) {
    char cwd[4096];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("trigger");
        return true;
    }

    printf("%s\n", cwd);
    return true;
}

int trigger_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        if (i > 1) {
            printf(" ");
        }
        printf("%s", args[i]);
    }
    printf("\n");
    return true;
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
    return true;
}

int trigger_unset(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        unsetenv(args[i]);
    }
    return true;
}
