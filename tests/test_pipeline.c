#include "../include/builtins.h"
#include "../include/execute.h"
#include "../include/input.h"
#include "../include/pipeline.h"
#include "../src/utils/utils.h"
#include "test_framework.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static TokenList *make_tokenlist(const char *input) {
    return parse_line_with_quotes(input);
}

static void free_parse_result(PipelineStage *stages, int num_stages) {
    trigger_free_pipeline(stages, num_stages);
}

static int run_command(const char *input) {
    TokenList *tl = parse_line_with_quotes(input);
    ASSERT_NOT_NULL(tl, input);
    ExecuteResult r = trigger_execute(tl);
    token_list_free(tl);
    return r.status == 0;
}

void test_parse_simple_pipe() {
    TokenList *tl = make_tokenlist("ls | grep src");
    ASSERT_NOT_NULL(tl, "tokens should not be NULL");

    int num_stages = 0;
    PipelineStage *stages = trigger_parse_pipeline(tl->argv, tl->glob_eligible, &num_stages);
    token_list_free(tl);

    ASSERT_EQUAL(2, num_stages, "should have 2 stages");
    ASSERT_STR_EQUAL("ls", stages[0].argv[0], "first stage command is ls");
    ASSERT_NULL(stages[0].argv[1], "first stage has one arg");
    ASSERT_STR_EQUAL("grep", stages[1].argv[0], "second stage command is grep");
    ASSERT_STR_EQUAL("src", stages[1].argv[1], "second stage has src arg");
    ASSERT_NULL(stages[1].argv[2], "second stage has two args");

    free_parse_result(stages, num_stages);
}

void test_parse_redirect_out() {
    TokenList *tl = make_tokenlist("echo hello > /tmp/out.txt");
    ASSERT_NOT_NULL(tl, "tokens should not be NULL");

    int num_stages = 0;
    PipelineStage *stages = trigger_parse_pipeline(tl->argv, tl->glob_eligible, &num_stages);
    token_list_free(tl);

    ASSERT_EQUAL(1, num_stages, "should have 1 stage");
    ASSERT_STR_EQUAL("echo", stages[0].argv[0], "command is echo");
    ASSERT_STR_EQUAL("hello", stages[0].argv[1], "arg is hello");
    ASSERT_NULL(stages[0].argv[2], "two args");
    ASSERT_STR_EQUAL("/tmp/out.txt", stages[0].outfile, "outfile is /tmp/out.txt");
    ASSERT_EQUAL(0, stages[0].append, "append should be 0");
    ASSERT_NULL(stages[0].infile, "no infile");

    free_parse_result(stages, num_stages);
}

void test_parse_redirect_append() {
    TokenList *tl = make_tokenlist("echo hello >> /tmp/out.txt");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    int num_stages = 0;
    PipelineStage *stages = trigger_parse_pipeline(tl->argv, tl->glob_eligible, &num_stages);
    token_list_free(tl);

    ASSERT_EQUAL(1, num_stages, "should have 1 stage");
    ASSERT_STR_EQUAL("/tmp/out.txt", stages[0].outfile, "outfile is /tmp/out.txt");
    ASSERT_EQUAL(1, stages[0].append, "append should be 1");

    free_parse_result(stages, num_stages);
}

void test_parse_redirect_in() {
    TokenList *tl = make_tokenlist("wc -l < /tmp/input.txt");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    int num_stages = 0;
    PipelineStage *stages = trigger_parse_pipeline(tl->argv, tl->glob_eligible, &num_stages);
    token_list_free(tl);

    ASSERT_EQUAL(1, num_stages, "should have 1 stage");
    ASSERT_STR_EQUAL("wc", stages[0].argv[0], "command is wc");
    ASSERT_STR_EQUAL("-l", stages[0].argv[1], "arg is -l");
    ASSERT_NULL(stages[0].argv[2], "two args");
    ASSERT_STR_EQUAL("/tmp/input.txt", stages[0].infile, "infile is /tmp/input.txt");
    ASSERT_NULL(stages[0].outfile, "no outfile");

    free_parse_result(stages, num_stages);
}

void test_parse_pipe_with_redirect() {
    TokenList *tl = make_tokenlist("cat < /etc/hostname | wc -c > /tmp/count.txt");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    int num_stages = 0;
    PipelineStage *stages = trigger_parse_pipeline(tl->argv, tl->glob_eligible, &num_stages);
    token_list_free(tl);

    ASSERT_EQUAL(2, num_stages, "should have 2 stages");
    ASSERT_STR_EQUAL("cat", stages[0].argv[0], "first stage: cat");
    ASSERT_STR_EQUAL("/etc/hostname", stages[0].infile, "first stage: infile");
    ASSERT_NULL(stages[0].outfile, "first stage: no outfile");
    ASSERT_STR_EQUAL("wc", stages[1].argv[0], "second stage: wc");
    ASSERT_STR_EQUAL("-c", stages[1].argv[1], "second stage: -c");
    ASSERT_STR_EQUAL("/tmp/count.txt", stages[1].outfile, "second stage: outfile");
    ASSERT_NULL(stages[1].infile, "second stage: no infile");

    free_parse_result(stages, num_stages);
}

void test_e2e_redirect_out() {
    unlink("/tmp/trigger_t_e2e_out");

    int result = run_command("/bin/echo test_output > /tmp/trigger_t_e2e_out");
    ASSERT_TRUE(result, "echo with redirect should return true");

    FILE *f = fopen("/tmp/trigger_t_e2e_out", "r");
    ASSERT_NOT_NULL(f, "output file should exist");

    char buf[256];
    ASSERT_NOT_NULL(fgets(buf, sizeof(buf), f), "should read from output file");
    fclose(f);

    ASSERT_STR_EQUAL("test_output\n", buf, "output file should contain test_output");
    unlink("/tmp/trigger_t_e2e_out");
}

void test_e2e_redirect_append() {
    unlink("/tmp/trigger_t_e2e_append");

    int result = run_command("/bin/echo first > /tmp/trigger_t_e2e_append");
    ASSERT_TRUE(result, "first redirect should succeed");

    result = run_command("/bin/echo second >> /tmp/trigger_t_e2e_append");
    ASSERT_TRUE(result, "append redirect should succeed");

    FILE *f = fopen("/tmp/trigger_t_e2e_append", "r");
    ASSERT_NOT_NULL(f, "append file should exist");

    char b1[256], b2[256];
    ASSERT_NOT_NULL(fgets(b1, sizeof(b1), f), "should read first line");
    ASSERT_NOT_NULL(fgets(b2, sizeof(b2), f), "should read second line");
    fclose(f);

    ASSERT_STR_EQUAL("first\n", b1, "first line should be first");
    ASSERT_STR_EQUAL("second\n", b2, "second line should be second");
    unlink("/tmp/trigger_t_e2e_append");
}

void test_e2e_redirect_in() {
    FILE *f = fopen("/tmp/trigger_t_e2e_in", "w");
    fprintf(f, "hello\nworld\n");
    fclose(f);

    int result = run_command("/usr/bin/wc -l < /tmp/trigger_t_e2e_in");
    ASSERT_TRUE(result, "input redirect should return true");
    unlink("/tmp/trigger_t_e2e_in");
}

void test_e2e_pipe() {
    int result = run_command("/bin/echo hello | /bin/cat");
    ASSERT_TRUE(result, "pipe should succeed");
}

void test_e2e_builtin_in_pipeline() {
    char cwd_before[4096];
    ASSERT_NOT_NULL(getcwd(cwd_before, sizeof(cwd_before)), "getcwd should succeed");

    int result = run_command("cd /tmp | /bin/true");
    ASSERT_TRUE(result, "cd in pipe should succeed");

    char cwd_after[4096];
    ASSERT_NOT_NULL(getcwd(cwd_after, sizeof(cwd_after)), "getcwd should succeed");
    ASSERT_STR_EQUAL(cwd_before, cwd_after, "cd in pipeline should not affect parent cwd");
}

void test_quoted_pipe_not_operator() {
    int result = run_command("/bin/echo \"|\"");
    ASSERT_TRUE(result, "echo of quoted pipe char should not be parsed as pipeline");
}

void test_quoted_redirect_not_operator() {
    int result = run_command("/bin/echo \">\"");
    ASSERT_TRUE(result, "echo of quoted > should not be parsed as redirect");
}

void test_parse_redirect_missing_filename() {
    TokenList *tl = make_tokenlist("cmd >");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    int num_stages = 0;
    PipelineStage *stages = trigger_parse_pipeline(tl->argv, tl->glob_eligible, &num_stages);
    token_list_free(tl);

    ASSERT_NULL(stages, "parse should return NULL on missing filename");
    ASSERT_EQUAL(0, num_stages, "num_stages should be 0 on syntax error");
}

void test_parse_redirect_no_arg_pipe() {
    TokenList *tl = make_tokenlist("cmd > | next");
    ASSERT_NOT_NULL(tl, "TokenList should not be NULL");

    int num_stages = 0;
    PipelineStage *stages = trigger_parse_pipeline(tl->argv, tl->glob_eligible, &num_stages);
    token_list_free(tl);

    ASSERT_NULL(stages, "parse should return NULL on >| syntax error");
    ASSERT_EQUAL(0, num_stages, "num_stages should be 0 on error");
}

void test_quoted_pipe_split_correctly() {
    int *glob_eligible = NULL;
    char **args = trigger_split_line_ex("/bin/echo \"|\" | /bin/cat", &glob_eligible);
    ASSERT_NOT_NULL(args, "args should not be NULL");

    int num_stages = 0;
    PipelineStage *stages = trigger_parse_pipeline(args, glob_eligible, &num_stages);
    free(args);
    free(glob_eligible);

    ASSERT_NOT_NULL(stages, "parse should succeed");
    ASSERT_EQUAL(2, num_stages, "should have 2 stages: echo with literal pipe | cat");
    ASSERT_STR_EQUAL("/bin/echo", stages[0].argv[0], "first stage cmd: echo");
    ASSERT_STR_EQUAL("|", stages[0].argv[1], "first stage arg: literal |");
    ASSERT_NULL(stages[0].argv[2], "first stage has 2 tokens");
    ASSERT_STR_EQUAL("/bin/cat", stages[1].argv[0], "second stage cmd: cat");
    ASSERT_NULL(stages[1].argv[1], "second stage has 1 token");

    trigger_free_pipeline(stages, num_stages);
}

int main() {
    TEST_SUITE_START("Pipeline Tests");

    test_parse_simple_pipe();
    test_parse_redirect_out();
    test_parse_redirect_append();
    test_parse_redirect_in();
    test_parse_pipe_with_redirect();
    test_e2e_redirect_out();
    test_e2e_redirect_append();
    test_e2e_redirect_in();
    test_e2e_pipe();
    test_e2e_builtin_in_pipeline();
    test_quoted_pipe_not_operator();
    test_quoted_redirect_not_operator();
    test_parse_redirect_missing_filename();
    test_parse_redirect_no_arg_pipe();
    test_quoted_pipe_split_correctly();

    TEST_SUITE_END();
}
