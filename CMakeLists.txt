cmake_minimum_required(VERSION 3.28.3)
project(shell C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

set(LIB_SOURCES
        src/input.c
        src/execute.c
        src/builtins.c
        src/pipeline.c
        src/glob.c
)

set(UTIL_SOURCES
        src/utils/utils.c
)

set(HEADERS
        include/input.h
        include/execute.h
        include/builtins.h
        include/pipeline.h
        include/glob_expand.h
)

add_executable(shell src/main.c ${LIB_SOURCES} ${UTIL_SOURCES} ${HEADERS})

enable_testing()

add_executable(test_input tests/test_input.c src/input.c ${UTIL_SOURCES})
add_test(NAME InputTests COMMAND test_input)

add_executable(test_builtins tests/test_builtins.c src/builtins.c src/input.c ${UTIL_SOURCES})
add_test(NAME BuiltinsTests COMMAND test_builtins)

add_executable(test_execute tests/test_execute.c src/execute.c src/builtins.c src/pipeline.c src/glob.c src/input.c ${UTIL_SOURCES})
add_test(NAME ExecuteTests COMMAND test_execute)

add_executable(test_pipeline tests/test_pipeline.c src/pipeline.c src/execute.c src/builtins.c src/glob.c src/input.c ${UTIL_SOURCES})
add_test(NAME PipelineTests COMMAND test_pipeline)

add_executable(test_glob tests/test_glob.c src/glob.c src/input.c ${UTIL_SOURCES})
add_test(NAME GlobTests COMMAND test_glob)

add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_input test_builtins test_execute test_pipeline test_glob
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
