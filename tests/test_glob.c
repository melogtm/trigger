#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/glob_expand.h"
#include "../include/input.h"
#include "../include/pipeline.h"
#include "../src/utils/utils.h"
#include "test_framework.h"

static void setup_fixture() {
    mkdir("/tmp/trigger_glob_test", 0755);
    FILE *f = fopen("/tmp/trigger_glob_test/a.txt", "w"); if (f) fclose(f);
    f = fopen("/tmp/trigger_glob_test/b.txt", "w"); if (f) fclose(f);
    f = fopen("/tmp/trigger_glob_test/c.md", "w"); if (f) fclose(f);
    f = fopen("/tmp/trigger_glob_test/d.dat", "w"); if (f) fclose(f);
    f = fopen("/tmp/trigger_glob_test/ax.txt", "w"); if (f) fclose(f);
}

static void teardown_fixture() {
    unlink("/tmp/trigger_glob_test/a.txt");
    unlink("/tmp/trigger_glob_test/b.txt");
    unlink("/tmp/trigger_glob_test/c.md");
    unlink("/tmp/trigger_glob_test/d.dat");
    unlink("/tmp/trigger_glob_test/ax.txt");
    rmdir("/tmp/trigger_glob_test");
}

void test_glob_txt_files() {
    setup_fixture();

    int *glob_eligible = NULL;
    char **args = trigger_split_line_ex("echo /tmp/trigger_glob_test/*.txt", &glob_eligible);
    ASSERT_NOT_NULL(args, "args should not be NULL");
    ASSERT_NOT_NULL(glob_eligible, "glob_eligible should not be NULL");

    ASSERT_TRUE(glob_eligible[1], "*.txt is glob-eligible (unquoted)");

    char **expanded = expand_globs(args, &glob_eligible);

    int count = 0;
    while (expanded[count] != NULL) count++;
    ASSERT_EQUAL(4, count, "should have 4 entries: echo + a.txt + b.txt + ax.txt");

    int found_a = 0, found_b = 0, found_ax = 0;
    for (int i = 0; expanded[i] != NULL; i++) {
        if (strstr(expanded[i], "a.txt")) found_a = 1;
        if (strstr(expanded[i], "b.txt")) found_b = 1;
        if (strstr(expanded[i], "ax.txt")) found_ax = 1;
    }
    ASSERT_TRUE(found_a, "should have a.txt in expansion");
    ASSERT_TRUE(found_b, "should have b.txt in expansion");
    ASSERT_TRUE(found_ax, "should have ax.txt in expansion");

    ASSERT_NOT_NULL(glob_eligible, "glob_eligible should still be valid after expansion");

    free(glob_eligible);
    free_array_of_strings(expanded);
    teardown_fixture();
}

void test_quoted_glob_not_expanded() {
    setup_fixture();

    int *glob_eligible = NULL;
    char **args = trigger_split_line_ex("echo '/tmp/trigger_glob_test/*.txt'", &glob_eligible);
    ASSERT_NOT_NULL(args, "args should not be NULL");

    ASSERT_FALSE(glob_eligible[1], "quoted *.txt is NOT glob-eligible");

    char **expanded = expand_globs(args, &glob_eligible);

    ASSERT_TRUE(expanded == args, "should return args unchanged (no eligible tokens)");

    ASSERT_STR_EQUAL("/tmp/trigger_glob_test/*.txt", expanded[1],
                     "quoted glob should stay as literal");

    free(glob_eligible);
    free_array_of_strings(expanded);
    teardown_fixture();
}

void test_no_match_pattern() {
    setup_fixture();

    int *glob_eligible = NULL;
    char **args = trigger_split_line_ex("echo /tmp/trigger_glob_test/*.nonexistent", &glob_eligible);
    ASSERT_NOT_NULL(args, "args should not be NULL");

    char **expanded = expand_globs(args, &glob_eligible);

    ASSERT_STR_EQUAL("/tmp/trigger_glob_test/*.nonexistent", expanded[1],
                     "no-match pattern should pass through literally");

    free(glob_eligible);
    free_array_of_strings(expanded);
    teardown_fixture();
}

void test_question_mark_glob() {
    setup_fixture();

    int *glob_eligible = NULL;
    char **args = trigger_split_line_ex("echo /tmp/trigger_glob_test/?.txt", &glob_eligible);
    ASSERT_NOT_NULL(args, "args should not be NULL");

    char **expanded = expand_globs(args, &glob_eligible);

    ASSERT_STR_EQUAL("echo", expanded[0], "command is echo");
    ASSERT_STR_EQUAL("/tmp/trigger_glob_test/a.txt", expanded[1], "?.txt matches a.txt");
    ASSERT_STR_EQUAL("/tmp/trigger_glob_test/b.txt", expanded[2], "?.txt matches b.txt");
    ASSERT_NULL(expanded[3], "should not match ax.txt (two chars)");

    free(glob_eligible);
    free_array_of_strings(expanded);
    teardown_fixture();
}

void test_no_glob_simple_args() {
    int *glob_eligible = NULL;
    char **args = trigger_split_line_ex("echo hello world", &glob_eligible);
    ASSERT_NOT_NULL(args, "args should not be NULL");

    char **expanded = expand_globs(args, &glob_eligible);

    ASSERT_TRUE(expanded == args, "args with no globs returns same pointer");

    free(glob_eligible);
    free_array_of_strings(expanded);
}

void test_escaped_glob_not_expanded() {
    setup_fixture();

    int *glob_eligible = NULL;
    char **args = trigger_split_line_ex("echo /tmp/trigger_glob_test/\\*.txt", &glob_eligible);
    ASSERT_NOT_NULL(args, "args should not be NULL");

    ASSERT_FALSE(glob_eligible[1], "backslash-escaped *.txt is NOT glob-eligible");

    free(glob_eligible);
    free_array_of_strings(args);
    teardown_fixture();
}

void test_glob_eligible_after_expansion() {
    setup_fixture();

    int *glob_eligible = NULL;
    char **args = trigger_split_line_ex("echo /tmp/trigger_glob_test/*.txt", &glob_eligible);
    ASSERT_NOT_NULL(args, "args should not be NULL");

    char **expanded = expand_globs(args, &glob_eligible);

    int count = 0;
    while (expanded[count] != NULL) count++;
    ASSERT_EQUAL(4, count, "should have 4 entries");

    ASSERT_TRUE(glob_eligible[0], "echo should keep original eligibility=1");

    for (int i = 1; i < 4; i++) {
        ASSERT_FALSE(glob_eligible[i], "glob-expanded filenames should be eligible=0");
    }

    free(glob_eligible);
    free_array_of_strings(expanded);
    teardown_fixture();
}

void test_glob_with_pipe_operator() {
    setup_fixture();

    int *glob_eligible = NULL;
    char **args = trigger_split_line_ex(
        "echo /tmp/trigger_glob_test/*.txt | wc x", &glob_eligible);
    ASSERT_NOT_NULL(args, "args should not be NULL");

    char **expanded = expand_globs(args, &glob_eligible);

    int count = 0;
    while (expanded[count] != NULL) count++;

    ASSERT_EQUAL(7, count, "should have 7 entries: echo a b ax | wc x");

    ASSERT_STR_EQUAL("echo", expanded[0], "command is echo");
    ASSERT_STR_EQUAL("|", expanded[4], "pipe at index 4 after glob expansion");
    ASSERT_STR_EQUAL("wc", expanded[5], "wc at index 5");
    ASSERT_STR_EQUAL("x", expanded[6], "x at index 6");

    ASSERT_TRUE(glob_eligible[4], "pipe operator after glob should be glob_eligible=1");

    free(glob_eligible);
    free_array_of_strings(expanded);
    teardown_fixture();
}

int main() {
    TEST_SUITE_START("Glob Expansion Tests");

    test_glob_txt_files();
    test_quoted_glob_not_expanded();
    test_no_match_pattern();
    test_question_mark_glob();
    test_no_glob_simple_args();
    test_escaped_glob_not_expanded();
    test_glob_eligible_after_expansion();
    test_glob_with_pipe_operator();

    TEST_SUITE_END();
}
