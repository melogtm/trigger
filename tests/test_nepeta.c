#include "../include/nepeta.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void reset_nepeta(void) { trigger_nepeta_set_enabled(false); }

void test_default_disabled() {
    reset_nepeta();
    ASSERT_FALSE(trigger_nepeta_is_enabled(), "Nepeta mode should be disabled by default");
}

void test_set_get_roundtrip() {
    reset_nepeta();
    trigger_nepeta_set_enabled(true);
    ASSERT_TRUE(trigger_nepeta_is_enabled(), "Should be enabled after set(true)");
    trigger_nepeta_set_enabled(false);
    ASSERT_FALSE(trigger_nepeta_is_enabled(), "Should be disabled after set(false)");
}

void test_prompt_disabled() {
    reset_nepeta();
    ASSERT_STR_EQUAL("> ", trigger_nepeta_prompt(), "Prompt should be default when disabled");
}

void test_prompt_enabled() {
    reset_nepeta();
    trigger_nepeta_set_enabled(true);
    ASSERT_STR_EQUAL(":33 < ", trigger_nepeta_prompt(), "Prompt should be :33 < when enabled");
}

void test_translate_known_word() {
    reset_nepeta();
    char *t = trigger_nepeta_translate("for");
    ASSERT_STR_EQUAL("fur", t, "\"for\" should translate to \"fur\"");
    free(t);
}

void test_translate_case_preserving() {
    reset_nepeta();
    char *t = trigger_nepeta_translate("For");
    ASSERT_STR_EQUAL("Fur", t, "\"For\" should translate to \"Fur\"");
    free(t);
}

void test_translate_no_substring_match() {
    reset_nepeta();
    char *t = trigger_nepeta_translate("information");
    ASSERT_STR_EQUAL("information", t, "\"information\" should not match \"for\" substring");
    free(t);
}

void test_translate_no_match_passthrough() {
    reset_nepeta();
    char *t = trigger_nepeta_translate("hello world");
    ASSERT_STR_EQUAL("hello world", t, "Unmatched text should pass through unchanged");
    free(t);
}

void test_translate_multiple_words() {
    reset_nepeta();
    char *t = trigger_nepeta_translate("this is great for a friend");
    ASSERT_STR_EQUAL("this is clawesome fur a furriend", t,
                     "Multiple words should all translate");
    free(t);
}

void test_translate_null() {
    reset_nepeta();
    char *t = trigger_nepeta_translate(NULL);
    ASSERT_NOT_NULL(t, "NULL input should return allocated empty string");
    ASSERT_STR_EQUAL("", t, "NULL input should return empty string");
    free(t);
}

void test_translate_empty() {
    reset_nepeta();
    char *t = trigger_nepeta_translate("");
    ASSERT_STR_EQUAL("", t, "Empty input should return empty string");
    free(t);
}

static char *capture_say(const char *text) {
    char *buf;
    size_t sz;
    FILE *f = open_memstream(&buf, &sz);
    if (f == NULL)
        return NULL;
    trigger_nepeta_say(f, text);
    fclose(f);
    return buf;
}

void test_say_disabled() {
    reset_nepeta();
    char *buf = capture_say("hello");
    ASSERT_NOT_NULL(buf, "capture_say should not return NULL when disabled");
    ASSERT_STR_EQUAL("hello", buf, "say should pass through unchanged when disabled");
    free(buf);
}

void test_say_enabled() {
    reset_nepeta();
    trigger_nepeta_set_enabled(true);
    char *buf = capture_say("this is for you");
    ASSERT_NOT_NULL(buf, "capture_say should not return NULL when enabled");
    ASSERT_STR_EQUAL(":33 < this is fur you", buf,
                     "say should prefix with :33 < and translate when enabled");
    free(buf);
}

void test_random_quote_not_null() {
    reset_nepeta();
    const char *q = trigger_nepeta_random_quote();
    ASSERT_NOT_NULL(q, "Random quote should not be NULL");
    ASSERT_TRUE(strlen(q) > 0, "Random quote should not be empty");
}

void test_init_from_env_enabled() {
    reset_nepeta();
    setenv("TRIGGER_NEPETA", "1", 1);
    trigger_nepeta_init_from_env();
    ASSERT_TRUE(trigger_nepeta_is_enabled(), "Should enable when TRIGGER_NEPETA=1");
    unsetenv("TRIGGER_NEPETA");
}

void test_init_from_env_disabled_zero() {
    reset_nepeta();
    setenv("TRIGGER_NEPETA", "0", 1);
    trigger_nepeta_init_from_env();
    ASSERT_FALSE(trigger_nepeta_is_enabled(), "Should stay disabled when TRIGGER_NEPETA=0");
    unsetenv("TRIGGER_NEPETA");
}

void test_init_from_env_disabled_unset() {
    reset_nepeta();
    unsetenv("TRIGGER_NEPETA");
    trigger_nepeta_init_from_env();
    ASSERT_FALSE(trigger_nepeta_is_enabled(), "Should stay disabled when TRIGGER_NEPETA unset");
}

void test_init_from_env_disabled_empty() {
    reset_nepeta();
    setenv("TRIGGER_NEPETA", "", 1);
    trigger_nepeta_init_from_env();
    ASSERT_FALSE(trigger_nepeta_is_enabled(), "Should stay disabled when TRIGGER_NEPETA=\"\"");
    unsetenv("TRIGGER_NEPETA");
}

int main() {
    TEST_SUITE_START("Nepeta Module Tests");

    test_default_disabled();
    test_set_get_roundtrip();
    test_prompt_disabled();
    test_prompt_enabled();
    test_translate_known_word();
    test_translate_case_preserving();
    test_translate_no_substring_match();
    test_translate_no_match_passthrough();
    test_translate_multiple_words();
    test_translate_null();
    test_translate_empty();
    test_say_disabled();
    test_say_enabled();
    test_random_quote_not_null();
    test_init_from_env_enabled();
    test_init_from_env_disabled_zero();
    test_init_from_env_disabled_unset();
    test_init_from_env_disabled_empty();

    TEST_SUITE_END();
}
