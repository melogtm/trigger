#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "builtins.h"

#define true 1

char *builtin_str[] = {"cd", "help", "exit"};

/*
 * builtin_func is an array of pointers to functions that take a char** as an argument and return an int.
 * This is the devil.
 */
int (*builtin_func[])(char **) = {&trigger_cd, &trigger_help, &trigger_exit};

int trigger_num_builtins(void) {
    return sizeof(builtin_str) / sizeof(char *);
}

int trigger_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "trigger: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
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

