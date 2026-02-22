#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/input.h"
#include "test_framework.h"
#include "../src/utils/utils.h"

void test_split_line_basic() {
    char *line = strdup("ls -la /home");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("ls", tokens[0], "First token should be 'ls'");
    ASSERT_STR_EQUAL("-la", tokens[1], "Second token should be '-la'");
    ASSERT_STR_EQUAL("/home", tokens[2], "Third token should be '/home'");
    ASSERT_NULL(tokens[3], "Fourth token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_empty() {
    char *line = strdup("   \t\n  ");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL for empty input");
    ASSERT_NULL(tokens[0], "First token should be NULL for empty input");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_single_command() {
    char *line = strdup("pwd");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("pwd", tokens[0], "First token should be 'pwd'");
    ASSERT_NULL(tokens[1], "Second token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_multiple_spaces() {
    char *line = strdup("echo    hello     world");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello", tokens[1], "Second token should be 'hello'");
    ASSERT_STR_EQUAL("world", tokens[2], "Third token should be 'world'");
    ASSERT_NULL(tokens[3], "Fourth token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_tabs() {
    char *line = strdup("cat\tfile.txt\tfile2.txt");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("cat", tokens[0], "First token should be 'cat'");
    ASSERT_STR_EQUAL("file.txt", tokens[1], "Second token should be 'file.txt'");
    ASSERT_STR_EQUAL("file2.txt", tokens[2], "Third token should be 'file2.txt'");
    ASSERT_NULL(tokens[3], "Fourth token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

// ==================== DOUBLE QUOTE TESTS ====================

void test_split_line_double_quotes_basic() {
    char *line = strdup("echo \"hello world\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello world", tokens[1], "Second token should be 'hello world' (with space preserved)");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_double_quotes_empty() {
    char *line = strdup("echo \"\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("", tokens[1], "Second token should be empty string");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_double_quotes_multiple() {
    char *line = strdup("echo \"hello\" \"world\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello", tokens[1], "Second token should be 'hello'");
    ASSERT_STR_EQUAL("world", tokens[2], "Third token should be 'world'");
    ASSERT_NULL(tokens[3], "Fourth token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_double_quotes_with_tabs() {
    char *line = strdup("echo \"hello\tworld\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello\tworld", tokens[1], "Second token should preserve tab character");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_double_quotes_concatenated() {
    char *line = strdup("echo hello\"world\"test");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("helloworldtest", tokens[1], "Second token should be 'helloworldtest' (concatenated)");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

// ==================== SINGLE QUOTE TESTS ====================

void test_split_line_single_quotes_basic() {
    char *line = strdup("echo 'hello world'");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello world", tokens[1], "Second token should be 'hello world' (with space preserved)");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_single_quotes_empty() {
    char *line = strdup("echo ''");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("", tokens[1], "Second token should be empty string");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_single_quotes_with_double_quotes() {
    char *line = strdup("echo 'hello \"world\"'");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello \"world\"", tokens[1], "Second token should preserve double quotes inside single quotes");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_single_quotes_concatenated() {
    char *line = strdup("echo test'hello world'end");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("testhello worldend", tokens[1], "Second token should be concatenated");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

// ==================== BACKSLASH ESCAPING TESTS ====================

void test_split_line_backslash_space() {
    char *line = strdup("echo hello\\ world");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello world", tokens[1], "Second token should be 'hello world' (escaped space)");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_backslash_multiple_spaces() {
    char *line = strdup("echo hello\\ \\ world");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello  world", tokens[1], "Second token should have two spaces");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_backslash_quote() {
    char *line = strdup("echo \\\"hello\\\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("\"hello\"", tokens[1], "Second token should contain literal quotes");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_backslash_single_quote() {
    char *line = strdup("echo \\'hello\\'");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("'hello'", tokens[1], "Second token should contain literal single quotes");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_backslash_backslash() {
    char *line = strdup("echo \\\\test");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("\\test", tokens[1], "Second token should contain literal backslash");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_backslash_tab() {
    char *line = strdup("echo hello\\\tworld");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello\tworld", tokens[1], "Second token should contain escaped tab");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_backslash_newline() {
    char *line = strdup("echo hello\\\nworld");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello\nworld", tokens[1], "Second token should contain escaped newline");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

// ==================== MIXED QUOTING TESTS ====================

void test_split_line_mixed_quotes() {
    char *line = strdup("echo \"hello 'world'\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello 'world'", tokens[1], "Second token should preserve single quotes in double quotes");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_mixed_quotes_reversed() {
    char *line = strdup("echo 'hello \"world\"'");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello \"world\"", tokens[1], "Second token should preserve double quotes in single quotes");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_quotes_and_backslash() {
    char *line = strdup("echo \"hello\\ world\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello world", tokens[1], "Second token should have escaped space in quotes");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_complex_mixed() {
    char *line = strdup("cmd arg1 \"arg 2\" 'arg 3' arg\\ 4");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("cmd", tokens[0], "First token should be 'cmd'");
    ASSERT_STR_EQUAL("arg1", tokens[1], "Second token should be 'arg1'");
    ASSERT_STR_EQUAL("arg 2", tokens[2], "Third token should be 'arg 2'");
    ASSERT_STR_EQUAL("arg 3", tokens[3], "Fourth token should be 'arg 3'");
    ASSERT_STR_EQUAL("arg 4", tokens[4], "Fifth token should be 'arg 4'");
    ASSERT_NULL(tokens[5], "Sixth token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

// ==================== EDGE CASES ====================

void test_split_line_adjacent_quotes() {
    char *line = strdup("echo \"hello\"\"world\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("helloworld", tokens[1], "Second token should be 'helloworld'");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_quote_at_start() {
    char *line = strdup("\"echo\" hello");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("hello", tokens[1], "Second token should be 'hello'");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_only_quotes() {
    char *line = strdup("\"\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("", tokens[0], "First token should be empty string");
    ASSERT_NULL(tokens[1], "Second token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_backslash_at_end() {
    char *line = strdup("echo test\\");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("test", tokens[1], "Second token should be 'test' (trailing backslash ignored or handled)");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_special_characters_in_quotes() {
    char *line = strdup("echo \"!@#$%^&*()\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("echo", tokens[0], "First token should be 'echo'");
    ASSERT_STR_EQUAL("!@#$%^&*()", tokens[1], "Second token should preserve special characters");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_path_with_spaces() {
    char *line = strdup("cd \"/home/user/my documents\"");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("cd", tokens[0], "First token should be 'cd'");
    ASSERT_STR_EQUAL("/home/user/my documents", tokens[1], "Second token should preserve path with spaces");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

void test_split_line_filename_with_escaped_spaces() {
    char *line = strdup("cat my\\ file\\ name.txt");
    char **tokens = trigger_split_line(line);

    ASSERT_NOT_NULL(tokens, "trigger_split_line should return non-NULL");
    ASSERT_STR_EQUAL("cat", tokens[0], "First token should be 'cat'");
    ASSERT_STR_EQUAL("my file name.txt", tokens[1], "Second token should be filename with spaces");
    ASSERT_NULL(tokens[2], "Third token should be NULL");

    free_array_of_strings(tokens);
    free(line);
}

int main() {
    TEST_SUITE_START("Input Module Tests");

    // Basic tests
    test_split_line_basic();
    test_split_line_empty();
    test_split_line_single_command();
    test_split_line_multiple_spaces();
    test_split_line_tabs();

    // Double quote tests
    test_split_line_double_quotes_basic();
    test_split_line_double_quotes_empty();
    test_split_line_double_quotes_multiple();
    test_split_line_double_quotes_with_tabs();
    test_split_line_double_quotes_concatenated();

    // Single quote tests
    test_split_line_single_quotes_basic();
    test_split_line_single_quotes_empty();
    test_split_line_single_quotes_with_double_quotes();
    test_split_line_single_quotes_concatenated();

    // Backslash escaping tests
    test_split_line_backslash_space();
    test_split_line_backslash_multiple_spaces();
    test_split_line_backslash_quote();
    test_split_line_backslash_single_quote();
    test_split_line_backslash_backslash();
    test_split_line_backslash_tab();
    test_split_line_backslash_newline();

    // Mixed quoting tests
    test_split_line_mixed_quotes();
    test_split_line_mixed_quotes_reversed();
    test_split_line_quotes_and_backslash();
    test_split_line_complex_mixed();

    // Edge cases
    test_split_line_adjacent_quotes();
    test_split_line_quote_at_start();
    test_split_line_only_quotes();
    test_split_line_backslash_at_end();
    test_split_line_special_characters_in_quotes();
    test_split_line_path_with_spaces();
    test_split_line_filename_with_escaped_spaces();

    TEST_SUITE_END();
}

