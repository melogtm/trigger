#include "../include/builtins.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void test_builtin_arrays_match() {
    int count = trigger_num_builtins();

    ASSERT_TRUE(count == 7, "Should have 7 built-in commands");

    ASSERT_NOT_NULL(find_builtin("cd"), "Should find 'cd' built-in");
    ASSERT_NOT_NULL(find_builtin("help"), "Should find 'help' built-in");
    ASSERT_NOT_NULL(find_builtin("exit"), "Should find 'exit' built-in");
    ASSERT_NOT_NULL(find_builtin("pwd"), "Should find 'pwd' built-in");
    ASSERT_NOT_NULL(find_builtin("echo"), "Should find 'echo' built-in");
    ASSERT_NOT_NULL(find_builtin("export"), "Should find 'export' built-in");
    ASSERT_NOT_NULL(find_builtin("unset"), "Should find 'unset' built-in");

    ASSERT_NULL(find_builtin("nonexistent"), "Should not find unknown built-in");

    const Builtin *cd_b = find_builtin("cd");
    ASSERT_NOT_NULL(cd_b, "cd Builtin* should not be NULL");
    ASSERT_TRUE(cd_b->fn == trigger_cd, "cd Builtin should point to trigger_cd");
}

void test_cd_no_args() {
    char *args[] = {"cd", NULL};
    int result = trigger_cd(args);

    ASSERT_EQUAL(1, result, "cd with no args should return 1 (error)");
}

void test_cd_to_tmp() {
    char cwd_before[1024];
    char cwd_after[1024];

    getcwd(cwd_before, sizeof(cwd_before));

    char *args[] = {"cd", "/tmp", NULL};
    int result = trigger_cd(args);

    getcwd(cwd_after, sizeof(cwd_after));

    ASSERT_EQUAL(0, result, "cd to /tmp should return 0");
    ASSERT_STR_EQUAL("/tmp", cwd_after, "Should be in /tmp directory");

    chdir(cwd_before);
}

void test_help_command() {
    char *args[] = {"help", NULL};
    int result = trigger_help(args);

    ASSERT_EQUAL(0, result, "help command should return 0");
}

void test_exit_command() {
    char *args[] = {"exit", NULL};
    int result = trigger_exit(args);

    ASSERT_EQUAL(EXIT_SUCCESS, result, "exit command should return EXIT_SUCCESS");
}

void test_exit_with_code() {
    char *args[] = {"exit", "3", NULL};
    int result = trigger_exit(args);

    ASSERT_EQUAL(3, result, "exit 3 should return 3");
}

void test_pwd_command() {
    char *args[] = {"pwd", NULL};

    char cwd_before[4096];
    getcwd(cwd_before, sizeof(cwd_before));

    int result = trigger_pwd(args);
    ASSERT_EQUAL(0, result, "pwd should return 0");

    char cwd_after[4096];
    getcwd(cwd_after, sizeof(cwd_after));
    ASSERT_STR_EQUAL(cwd_before, cwd_after, "pwd should not change current directory");
}

void test_echo_command() {
    char *args[] = {"echo", "hello", "world", NULL};
    int result = trigger_echo(args);
    ASSERT_EQUAL(0, result, "echo should return 0");
}

void test_echo_empty() {
    char *args[] = {"echo", NULL};
    int result = trigger_echo(args);
    ASSERT_EQUAL(0, result, "echo with no args should return 0");
}

void test_export_command() {
    char *args[] = {"export", "TEST_VAR=hello", NULL};
    int result = trigger_export(args);
    ASSERT_EQUAL(0, result, "export should return 0");
    ASSERT_STR_EQUAL("hello", getenv("TEST_VAR"), "exported variable should be available");
    unsetenv("TEST_VAR");
}

void test_export_no_value() {
    char *args[] = {"export", "NOVAL", NULL};
    int result = trigger_export(args);
    ASSERT_EQUAL(0, result, "export of invalid format should still return 0");
}

void test_unset_command() {
    setenv("UNSET_VAR", "test_value", 1);
    char *args[] = {"unset", "UNSET_VAR", NULL};
    int result = trigger_unset(args);
    ASSERT_EQUAL(0, result, "unset should return 0");
    ASSERT_NULL(getenv("UNSET_VAR"), "unset variable should not be available");
}

int main() {
    TEST_SUITE_START("Built-ins Module Tests");

    test_builtin_arrays_match();
    test_cd_no_args();
    test_cd_to_tmp();
    test_help_command();
    test_exit_command();
    test_exit_with_code();
    test_pwd_command();
    test_echo_command();
    test_echo_empty();
    test_export_command();
    test_export_no_value();
    test_unset_command();

    TEST_SUITE_END();
}
