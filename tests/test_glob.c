#include "../include/glob_expand.h"
#include "../include/input.h"
#include "../src/utils/utils.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void setup_fixture() {
    mkdir("/tmp/trigger_glob_test", 0755);
    FILE *f = fopen("/tmp/trigger_glob_test/a.txt", "w");
    if (f)
        fclose(f);
    f = fopen("/tmp/trigger_glob_test/b.txt", "w");
    if (f)
        fclose(f);
    f = fopen("/tmp/trigger_glob_test/c.md", "w");
    if (f)
        fclose(f);
    f = fopen("/tmp/trigger_glob_test/d.dat", "w");
    if (f)
        fclose(f);
    f = fopen("/tmp/trigger_glob_test/ax.txt", "w");
    if (f)
        fclose(f);
}

static void teardown_fixture() {
    unlink("/tmp/trigger_glob_test/a.txt");
    unlink("/tmp/trigger_glob_test/b.txt");
    unlink("/tmp/trigger_glob_test/c.md");
    unlink("/tmp/trigger_glob_test/d.dat");
    unlink("/tmp/trigger_glob_test/ax.txt");
    rmdir("/tmp/trigger_glob_test");
}

static TokenList *make_tl(const char *line) {
    int *glob_eligible = NULL;
    char **argv = trigger_split_line_ex(line, &glob_eligible);
    if (argv == NULL) {
        return NULL;
    }

    TokenList *tl = malloc(sizeof(TokenList));
    tl->argv = argv;
    tl->glob_eligible = glob_eligible;
    tl->count = 0;
    while (argv[tl->count] != NULL)
        tl->count++;
    return tl;
}

static void free_tl(TokenList *tl) {
    free(tl->argv);
    free(tl->glob_eligible);
    free(tl);
}

void test_glob_txt_files() {
    setup_fixture();

    TokenList *tl = make_tl("echo /tmp/trigger_glob_test/*.txt");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");
    ASSERT_NOT_NULL(tl->glob_eligible, "glob_eligible should not be NULL");

    ASSERT_TRUE(tl->glob_eligible[1], "*.txt is glob-eligible (unquoted)");

    expand_globs(tl);

    int count = 0;
    while (tl->argv[count] != NULL)
        count++;
    ASSERT_EQUAL(4, count, "should have 4 entries: echo + a.txt + b.txt + ax.txt");

    int found_a = 0, found_b = 0, found_ax = 0;
    for (int i = 0; tl->argv[i] != NULL; i++) {
        if (strstr(tl->argv[i], "a.txt"))
            found_a = 1;
        if (strstr(tl->argv[i], "b.txt"))
            found_b = 1;
        if (strstr(tl->argv[i], "ax.txt"))
            found_ax = 1;
    }
    ASSERT_TRUE(found_a, "should have a.txt in expansion");
    ASSERT_TRUE(found_b, "should have b.txt in expansion");
    ASSERT_TRUE(found_ax, "should have ax.txt in expansion");

    ASSERT_NOT_NULL(tl->glob_eligible, "glob_eligible should still be valid after expansion");

    free_tl(tl);
    teardown_fixture();
}

void test_quoted_glob_not_expanded() {
    setup_fixture();

    TokenList *tl = make_tl("echo '/tmp/trigger_glob_test/*.txt'");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    ASSERT_FALSE(tl->glob_eligible[1], "quoted *.txt is NOT glob-eligible");

    char *before = tl->argv[1];
    expand_globs(tl);

    ASSERT_STR_EQUAL(before, tl->argv[1], "quoted glob should stay as literal");
    ASSERT_STR_EQUAL("/tmp/trigger_glob_test/*.txt", tl->argv[1],
                     "quoted glob should stay as literal");

    free_tl(tl);
    teardown_fixture();
}

void test_no_match_pattern() {
    setup_fixture();

    TokenList *tl = make_tl("echo /tmp/trigger_glob_test/*.nonexistent");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    expand_globs(tl);

    ASSERT_STR_EQUAL("/tmp/trigger_glob_test/*.nonexistent", tl->argv[1],
                     "no-match pattern should pass through literally");

    free_tl(tl);
    teardown_fixture();
}

void test_question_mark_glob() {
    setup_fixture();

    TokenList *tl = make_tl("echo /tmp/trigger_glob_test/?.txt");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    expand_globs(tl);

    ASSERT_STR_EQUAL("echo", tl->argv[0], "command is echo");
    ASSERT_STR_EQUAL("/tmp/trigger_glob_test/a.txt", tl->argv[1], "?.txt matches a.txt");
    ASSERT_STR_EQUAL("/tmp/trigger_glob_test/b.txt", tl->argv[2], "?.txt matches b.txt");
    ASSERT_NULL(tl->argv[3], "should not match ax.txt (two chars)");

    free_tl(tl);
    teardown_fixture();
}

void test_no_glob_simple_args() {
    TokenList *tl = make_tl("echo hello world");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    expand_globs(tl);

    ASSERT_STR_EQUAL("echo", tl->argv[0], "command is echo");
    ASSERT_STR_EQUAL("hello", tl->argv[1], "second arg is hello");
    ASSERT_STR_EQUAL("world", tl->argv[2], "third arg is world");

    free_tl(tl);
}

void test_escaped_glob_not_expanded() {
    setup_fixture();

    TokenList *tl = make_tl("echo /tmp/trigger_glob_test/\\*.txt");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    ASSERT_FALSE(tl->glob_eligible[1], "backslash-escaped *.txt is NOT glob-eligible");

    free_tl(tl);
    teardown_fixture();
}

void test_glob_eligible_after_expansion() {
    setup_fixture();

    TokenList *tl = make_tl("echo /tmp/trigger_glob_test/*.txt");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    expand_globs(tl);

    int count = 0;
    while (tl->argv[count] != NULL)
        count++;
    ASSERT_EQUAL(4, count, "should have 4 entries");

    ASSERT_TRUE(tl->glob_eligible[0], "echo should keep original eligibility=1");

    for (int i = 1; i < 4; i++) {
        ASSERT_FALSE(tl->glob_eligible[i], "glob-expanded filenames should be eligible=0");
    }

    free_tl(tl);
    teardown_fixture();
}

void test_glob_with_pipe_operator() {
    setup_fixture();

    TokenList *tl = make_tl("echo /tmp/trigger_glob_test/*.txt | wc x");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    expand_globs(tl);

    int count = 0;
    while (tl->argv[count] != NULL)
        count++;

    ASSERT_EQUAL(7, count, "should have 7 entries: echo a b ax | wc x");

    ASSERT_STR_EQUAL("echo", tl->argv[0], "command is echo");
    ASSERT_STR_EQUAL("|", tl->argv[4], "pipe at index 4 after glob expansion");
    ASSERT_STR_EQUAL("wc", tl->argv[5], "wc at index 5");
    ASSERT_STR_EQUAL("x", tl->argv[6], "x at index 6");

    ASSERT_TRUE(tl->glob_eligible[4], "pipe operator after glob should be glob_eligible=1");

    free_tl(tl);
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
