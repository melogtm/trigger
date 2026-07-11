#include "../include/builtins.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void test_builtin_arrays_match() {
    int count = trigger_num_builtins();

    ASSERT_TRUE(count == 7, "Should have 7 built-in commands");

    int found_cd = 0, found_help = 0, found_exit = 0;
    int found_pwd = 0, found_echo = 0, found_export = 0, found_unset = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(builtin_str[i], "cd") == 0)
            found_cd = 1;
        if (strcmp(builtin_str[i], "help") == 0)
            found_help = 1;
        if (strcmp(builtin_str[i], "exit") == 0)
            found_exit = 1;
        if (strcmp(builtin_str[i], "pwd") == 0)
            found_pwd = 1;
        if (strcmp(builtin_str[i], "echo") == 0)
            found_echo = 1;
        if (strcmp(builtin_str[i], "export") == 0)
            found_export = 1;
        if (strcmp(builtin_str[i], "unset") == 0)
            found_unset = 1;
    }

    ASSERT_TRUE(found_cd, "Should have 'cd' built-in");
    ASSERT_TRUE(found_help, "Should have 'help' built-in");
    ASSERT_TRUE(found_exit, "Should have 'exit' built-in");
    ASSERT_TRUE(found_pwd, "Should have 'pwd' built-in");
    ASSERT_TRUE(found_echo, "Should have 'echo' built-in");
    ASSERT_TRUE(found_export, "Should have 'export' built-in");
    ASSERT_TRUE(found_unset, "Should have 'unset' built-in");
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

void test_pwd_command() {
    char *args[] = {"pwd", NULL};

    char cwd_before[4096];
    getcwd(cwd_before, sizeof(cwd_before));

    int result = trigger_pwd(args);
    ASSERT_TRUE(result, "pwd should return true");

    char cwd_after[4096];
    getcwd(cwd_after, sizeof(cwd_after));
    ASSERT_STR_EQUAL(cwd_before, cwd_after, "pwd should not change current directory");
}

void test_echo_command() {
    char *args[] = {"echo", "hello", "world", NULL};
    int result = trigger_echo(args);
    ASSERT_TRUE(result, "echo should return true");
}

void test_echo_empty() {
    char *args[] = {"echo", NULL};
    int result = trigger_echo(args);
    ASSERT_TRUE(result, "echo with no args should return true");
}

void test_export_command() {
    char *args[] = {"export", "TEST_VAR=hello", NULL};
    int result = trigger_export(args);
    ASSERT_TRUE(result, "export should return true");
    ASSERT_STR_EQUAL("hello", getenv("TEST_VAR"), "exported variable should be available");
    unsetenv("TEST_VAR");
}

void test_export_no_value() {
    char *args[] = {"export", "NOVAL", NULL};
    int result = trigger_export(args);
    ASSERT_TRUE(result, "export of invalid format should still return true");
}

void test_unset_command() {
    setenv("UNSET_VAR", "test_value", 1);
    char *args[] = {"unset", "UNSET_VAR", NULL};
    int result = trigger_unset(args);
    ASSERT_TRUE(result, "unset should return true");
    ASSERT_NULL(getenv("UNSET_VAR"), "unset variable should not be available");
}

int main() {
    TEST_SUITE_START("Built-ins Module Tests");

    test_builtin_arrays_match();
    test_cd_no_args();
    test_cd_to_tmp();
    test_help_command();
    test_exit_command();
    test_pwd_command();
    test_echo_command();
    test_echo_empty();
    test_export_command();
    test_export_no_value();
    test_unset_command();

    TEST_SUITE_END();
}
