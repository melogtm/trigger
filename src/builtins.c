#include "builtins.h"
#include "nepeta.h"
#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const Builtin builtins[] = {
    {"cd", trigger_cd},
    {"help", trigger_help},
    {"exit", trigger_exit},
    {"pwd", trigger_pwd},
    {"echo", trigger_echo},
    {"export", trigger_export},
    {"unset", trigger_unset},
    {"nepeta", trigger_nepeta},
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
        trigger_nepeta_say(stderr, "trigger: expected argument to \"cd\"\n");
        return 1;
    }
    if (chdir(args[1]) != 0) {
        perror("trigger");
        return 1;
    }

    return 0;
}

int trigger_help(char **args __attribute__((unused))) {
    trigger_nepeta_say(stdout, "Trigger: A simple shell written in C\n");
    trigger_nepeta_say(stdout, "Type program names and arguments, and hit enter.\n");
    trigger_nepeta_say(stdout, "The following are built in:\n");

    for (int i = 0; i < builtin_count; i++) {
        printf("  %s\n", builtins[i].name);
    }

    trigger_nepeta_say(stdout, "Use the man command for information on other programs.\n");
    return 0;
}

int trigger_exit(char **args) {
    if (args[1] != NULL) {
        char *endptr;
        long n = strtol(args[1], &endptr, 10);
        if (*endptr == '\0' && n >= 0 && n <= 255) {
            return (int) n;
        }
        char buf[256];
        snprintf(buf, sizeof(buf), "trigger: exit: %s: numeric argument required\n", args[1]);
        trigger_nepeta_say(stderr, buf);
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

int trigger_nepeta(char **args) {
    if (args[1] == NULL) {
        trigger_nepeta_set_enabled(!trigger_nepeta_is_enabled());
        trigger_nepeta_say(stdout, "Nepeta mode toggled.\n");
        return 0;
    }

    if (strcmp(args[1], "on") == 0) {
        trigger_nepeta_set_enabled(true);
        trigger_nepeta_say(stdout, "Nepeta mode enabled.\n");
        return 0;
    }

    if (strcmp(args[1], "off") == 0) {
        trigger_nepeta_set_enabled(false);
        trigger_nepeta_say(stdout, "Nepeta mode disabled.\n");
        return 0;
    }

    if (strcmp(args[1], "status") == 0) {
        if (trigger_nepeta_is_enabled()) {
            trigger_nepeta_say(stdout, "Nepeta mode is enabled.\n");
        } else {
            trigger_nepeta_say(stdout, "Nepeta mode is disabled.\n");
        }
        return 0;
    }

    trigger_nepeta_say(stderr, "usage: nepeta [on|off|status]\n");
    return 1;
}
