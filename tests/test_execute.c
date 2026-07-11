#include "../include/execute.h"
#include "../include/utils/utils.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TokenList *make_tokenlist(int argc, char *words[]) {
    TokenList *tl = malloc(sizeof(TokenList));
    tl->argv = malloc((argc + 1) * sizeof(char *));
    int i;
    for (i = 0; i < argc; i++) {
        tl->argv[i] = words[i];
    }
    tl->argv[argc] = NULL;
    tl->glob_eligible = malloc((argc + 1) * sizeof(int));
    for (i = 0; i <= argc; i++) {
        tl->glob_eligible[i] = 1;
    }
    tl->count = argc;
    return tl;
}

static void free_tokenlist(TokenList *tl) {
    free(tl->argv);
    free(tl->glob_eligible);
    free(tl);
}

void test_execute_empty_command() {
    char *words[] = {NULL};
    TokenList *tl = make_tokenlist(0, words);
    ExecuteResult r = trigger_execute(tl);
    free_tokenlist(tl);

    ASSERT_FALSE(r.should_exit, "Empty command should not exit");
}

void test_execute_builtin_help() {
    char *words[] = {"help", NULL};
    TokenList *tl = make_tokenlist(1, words);
    ExecuteResult r = trigger_execute(tl);
    free_tokenlist(tl);

    ASSERT_EQUAL(0, r.status, "Execute 'help' should return status 0");
    ASSERT_FALSE(r.should_exit, "help should not exit");
}

void test_execute_builtin_exit() {
    char *words[] = {"exit", NULL};
    TokenList *tl = make_tokenlist(1, words);
    ExecuteResult r = trigger_execute(tl);
    free_tokenlist(tl);

    ASSERT_EQUAL(EXIT_SUCCESS, r.status, "Execute 'exit' should return EXIT_SUCCESS");
    ASSERT_TRUE(r.should_exit, "exit should set should_exit");
}

void test_execute_external_command() {
    char *words[] = {"/bin/true", NULL};
    TokenList *tl = make_tokenlist(1, words);
    ExecuteResult r = trigger_execute(tl);
    free_tokenlist(tl);

    ASSERT_FALSE(r.should_exit, "Execute '/bin/true' should not exit");
}

void test_launch_simple_command() {
    char *args[] = {"/bin/echo", "test", NULL};
    int result = trigger_launch(args);

    ASSERT_EQUAL(0, result, "Launch '/bin/echo test' should return status 0");
}

int main() {
    TEST_SUITE_START("Execute Module Tests");

    test_execute_empty_command();
    test_execute_builtin_help();
    test_execute_builtin_exit();
    test_execute_external_command();
    test_launch_simple_command();

    TEST_SUITE_END();
}
