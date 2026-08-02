#include "../include/builtins.h"
#include "../include/nepeta.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void test_builtin_arrays_match() {
    int count = trigger_num_builtins();

    ASSERT_TRUE(count == 8, "Should have 8 built-in commands");

    ASSERT_NOT_NULL(find_builtin("cd"), "Should find 'cd' built-in");
    ASSERT_NOT_NULL(find_builtin("help"), "Should find 'help' built-in");
    ASSERT_NOT_NULL(find_builtin("exit"), "Should find 'exit' built-in");
    ASSERT_NOT_NULL(find_builtin("pwd"), "Should find 'pwd' built-in");
    ASSERT_NOT_NULL(find_builtin("echo"), "Should find 'echo' built-in");
    ASSERT_NOT_NULL(find_builtin("export"), "Should find 'export' built-in");
    ASSERT_NOT_NULL(find_builtin("unset"), "Should find 'unset' built-in");
    ASSERT_NOT_NULL(find_builtin("nepeta"), "Should find 'nepeta' built-in");

    ASSERT_NULL(find_builtin("nonexistent"), "Should not find unknown built-in");

    const Builtin *cd_b = find_builtin("cd");
    ASSERT_NOT_NULL(cd_b, "cd Builtin* should not be NULL");
    ASSERT_TRUE(cd_b->fn == trigger_cd, "cd Builtin should point to trigger_cd");

    const Builtin *nepeta_b = find_builtin("nepeta");
    ASSERT_NOT_NULL(nepeta_b, "nepeta Builtin* should not be NULL");
    ASSERT_TRUE(nepeta_b->fn == trigger_nepeta, "nepeta Builtin should point to trigger_nepeta");
}

void test_cd_no_args() {
    char *args[] = {"cd", NULL};
    int result = trigger_cd(args);

    ASSERT_EQUAL(1, result, "cd with no args should return 1 (error)");
}

void test_cd_to_tmp() {
    char cwd_before[1024];
    char cwd_after[1024];

    ASSERT_NOT_NULL(getcwd(cwd_before, sizeof(cwd_before)), "getcwd before");

    char *args[] = {"cd", "/tmp", NULL};
    int result = trigger_cd(args);

    ASSERT_NOT_NULL(getcwd(cwd_after, sizeof(cwd_after)), "getcwd after");

    ASSERT_EQUAL(0, result, "cd to /tmp should return 0");
    ASSERT_STR_EQUAL("/tmp", cwd_after, "Should be in /tmp directory");

    ASSERT_EQUAL(0, chdir(cwd_before), "chdir back");
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
    ASSERT_NOT_NULL(getcwd(cwd_before, sizeof(cwd_before)), "getcwd before");

    int result = trigger_pwd(args);
    ASSERT_EQUAL(0, result, "pwd should return 0");

    char cwd_after[4096];
    ASSERT_NOT_NULL(getcwd(cwd_after, sizeof(cwd_after)), "getcwd after");
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

void test_nepeta_on() {
    trigger_nepeta_set_enabled(false);
    char *args[] = {"nepeta", "on", NULL};
    int result = trigger_nepeta(args);
    ASSERT_EQUAL(0, result, "nepeta on should return 0");
    ASSERT_TRUE(trigger_nepeta_is_enabled(), "Nepeta should be enabled after 'nepeta on'");
    trigger_nepeta_set_enabled(false);
}

void test_nepeta_off() {
    trigger_nepeta_set_enabled(true);
    char *args[] = {"nepeta", "off", NULL};
    int result = trigger_nepeta(args);
    ASSERT_EQUAL(0, result, "nepeta off should return 0");
    ASSERT_FALSE(trigger_nepeta_is_enabled(), "Nepeta should be disabled after 'nepeta off'");
    trigger_nepeta_set_enabled(false);
}

void test_nepeta_status() {
    trigger_nepeta_set_enabled(false);
    char *args[] = {"nepeta", "status", NULL};
    int result = trigger_nepeta(args);
    ASSERT_EQUAL(0, result, "nepeta status should return 0");
    ASSERT_FALSE(trigger_nepeta_is_enabled(), "Nepeta should still be disabled after status");
    trigger_nepeta_set_enabled(false);
}

void test_nepeta_bare_toggle() {
    trigger_nepeta_set_enabled(false);
    char *args[] = {"nepeta", NULL};
    int result = trigger_nepeta(args);
    ASSERT_EQUAL(0, result, "nepeta bare should return 0");
    ASSERT_TRUE(trigger_nepeta_is_enabled(), "Nepeta should be enabled after bare toggle");
    trigger_nepeta_set_enabled(false);
}

void test_nepeta_unknown_arg() {
    trigger_nepeta_set_enabled(false);
    char *args[] = {"nepeta", "meow", NULL};
    int result = trigger_nepeta(args);
    ASSERT_EQUAL(1, result, "nepeta with unknown arg should return 1");
    ASSERT_FALSE(trigger_nepeta_is_enabled(), "Nepeta should stay disabled after unknown arg");
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
    test_nepeta_on();
    test_nepeta_off();
    test_nepeta_status();
    test_nepeta_bare_toggle();
    test_nepeta_unknown_arg();

    TEST_SUITE_END();
}
