cmake_minimum_required(VERSION 3.28.3)
project(shell C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

set(LIB_SOURCES
        src/input.c
        src/execute.c
        src/builtins.c
)

set(HEADERS
        include/input.h
        include/execute.h
        include/builtins.h
)

add_executable(shell src/main.c ${LIB_SOURCES} ${HEADERS})

enable_testing()

add_executable(test_input tests/test_input.c src/input.c)
add_test(NAME InputTests COMMAND test_input)

add_executable(test_builtins tests/test_builtins.c src/builtins.c)
add_test(NAME BuiltinsTests COMMAND test_builtins)

add_executable(test_execute tests/test_execute.c src/execute.c src/builtins.c)
add_test(NAME ExecuteTests COMMAND test_execute)

add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_input test_builtins test_execute
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
