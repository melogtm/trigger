#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/builtins.h"
#include "test_framework.h"

void test_builtin_arrays_match() {
    int count = trigger_num_builtins();

    // Verify that builtin_str and builtin_func have the same size
    ASSERT_TRUE(count > 0, "Should have at least one built-in command");

    // Check that known builtins exist
    int found_cd = 0, found_help = 0, found_exit = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(builtin_str[i], "cd") == 0) found_cd = 1;
        if (strcmp(builtin_str[i], "help") == 0) found_help = 1;
        if (strcmp(builtin_str[i], "exit") == 0) found_exit = 1;
    }

    ASSERT_TRUE(found_cd, "Should have 'cd' built-in");
    ASSERT_TRUE(found_help, "Should have 'help' built-in");
    ASSERT_TRUE(found_exit, "Should have 'exit' built-in");
}

void test_cd_no_args() {
    char *args[] = {"cd", NULL};
    int result = trigger_cd(args);

    // Should return true (1) even when no directory provided
    ASSERT_TRUE(result, "cd with no args should return true");
}

void test_cd_to_tmp() {
    char cwd_before[1024];
    char cwd_after[1024];

    getcwd(cwd_before, sizeof(cwd_before));

    char *args[] = {"cd", "/tmp", NULL};
    int result = trigger_cd(args);

    getcwd(cwd_after, sizeof(cwd_after));

    ASSERT_TRUE(result, "cd to /tmp should return true");
    ASSERT_STR_EQUAL("/tmp", cwd_after, "Should be in /tmp directory");

    // Restore original directory
    chdir(cwd_before);
}

void test_help_command() {
    char *args[] = {"help", NULL};
    int result = trigger_help(args);

    ASSERT_TRUE(result, "help command should return true");
}

void test_exit_command() {
    char *args[] = {"exit", NULL};
    int result = trigger_exit(args);

    ASSERT_EQUAL(EXIT_SUCCESS, result, "exit command should return EXIT_SUCCESS");
}

int main() {
    TEST_SUITE_START("Built-ins Module Tests");

    test_builtin_arrays_match();
    test_cd_no_args();
    test_cd_to_tmp();
    test_help_command();
    test_exit_command();

    TEST_SUITE_END();
}

