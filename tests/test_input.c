#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/input.h"
#include "test_framework.h"

void test_split_line_basic() {
    char line[] = "ls -la /home";
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("ls", tokens[0], "First token should be 'ls'");
    ASSERT_STR_EQUAL("-la", tokens[1], "Second token should be '-la'");
    ASSERT_STR_EQUAL("/home", tokens[2], "Third token should be '/home'");
    ASSERT_NULL(tokens[3], "Fourth token should be NULL");

    free(tokens);
}

void test_split_line_empty() {
    char line[] = "   \t\n  ";
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL for empty input");
    ASSERT_NULL(tokens[0], "First token should be NULL for empty input");

    free(tokens);
}

void test_split_line_single_command() {
    char line[] = "pwd";
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("pwd", tokens[0], "First token should be 'pwd'");
    ASSERT_NULL(tokens[1], "Second token should be NULL");

    free(tokens);
}

void test_split_line_multiple_spaces() {
    char line[] = "echo    hello     world";
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello", tokens[1], "Second token should be 'hello'");
    ASSERT_STR_EQUAL("world", tokens[2], "Third token should be 'world'");
    ASSERT_NULL(tokens[3], "Fourth token should be NULL");

    free(tokens);
}

void test_split_line_tabs() {
    char line[] = "cat\tfile.txt\tfile2.txt";
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("cat", tokens[0], "First token should be 'cat'");
    ASSERT_STR_EQUAL("file.txt", tokens[1], "Second token should be 'file.txt'");
    ASSERT_STR_EQUAL("file2.txt", tokens[2], "Third token should be 'file2.txt'");
    ASSERT_NULL(tokens[3], "Fourth token should be NULL");

    free(tokens);
}

int main() {
    TEST_SUITE_START("Input Module Tests");

    test_split_line_basic();
    test_split_line_empty();
    test_split_line_single_command();
    test_split_line_multiple_spaces();
    test_split_line_tabs();

    TEST_SUITE_END();
}

