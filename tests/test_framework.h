#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

// Test statistics
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Color codes for output
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

// Assertion macros
#define ASSERT_TRUE(condition, message) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf(COLOR_GREEN "✓" COLOR_RESET " %s\n", message); \
        } else { \
            tests_failed++; \
            printf(COLOR_RED "✗" COLOR_RESET " %s (line %d)\n", message, __LINE__); \
        } \
    } while(0)

#define ASSERT_FALSE(condition, message) \
    ASSERT_TRUE(!(condition), message)

#define ASSERT_EQUAL(expected, actual, message) \
    ASSERT_TRUE((expected) == (actual), message)

#define ASSERT_STR_EQUAL(expected, actual, message) \
    ASSERT_TRUE(strcmp((expected), (actual)) == 0, message)

#define ASSERT_NOT_NULL(ptr, message) \
    ASSERT_TRUE((ptr) != NULL, message)

#define ASSERT_NULL(ptr, message) \
    ASSERT_TRUE((ptr) == NULL, message)

// Test suite management
#define TEST_SUITE_START(name) \
    printf("\n" COLOR_YELLOW "Running test suite: %s" COLOR_RESET "\n", name); \
    tests_run = 0; \
    tests_passed = 0; \
    tests_failed = 0;

#define TEST_SUITE_END() \
    printf("\n"); \
    if (tests_failed == 0) { \
        printf(COLOR_GREEN "All tests passed! (%d/%d)" COLOR_RESET "\n", tests_passed, tests_run); \
    } else { \
        printf(COLOR_RED "%d test(s) failed out of %d" COLOR_RESET "\n", tests_failed, tests_run); \
    } \
    printf("\n"); \
    return tests_failed;

#endif // TEST_FRAMEWORK_H

