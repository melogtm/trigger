#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/execute.h"
#include "test_framework.h"

void test_execute_empty_command() {
    char *args[] = {NULL};
    int result = trigger_execute(args);

    ASSERT_TRUE(result, "Empty command should return true");
}

void test_execute_builtin_help() {
    char *args[] = {"help", NULL};
    int result = trigger_execute(args);

    ASSERT_TRUE(result, "Execute 'help' should return true");
}

void test_execute_builtin_exit() {
    char *args[] = {"exit", NULL};
    int result = trigger_execute(args);

    ASSERT_EQUAL(EXIT_SUCCESS, result, "Execute 'exit' should return EXIT_SUCCESS");
}

void test_execute_external_command() {
    // Test with 'true' command which always succeeds
    char *args[] = {"/bin/true", NULL};
    int result = trigger_execute(args);

    ASSERT_TRUE(result, "Execute '/bin/true' should return true");
}

void test_launch_simple_command() {
    // Test launching echo command
    char *args[] = {"/bin/echo", "test", NULL};
    int result = trigger_launch(args);

    ASSERT_TRUE(result, "Launch '/bin/echo test' should return true");
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

