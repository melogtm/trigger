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

add_library(trigger_core STATIC ${LIB_SOURCES} ${UTIL_SOURCES})

option(TRIGGER_SANITIZE "Build with address and undefined behavior sanitizers" OFF)

if(TRIGGER_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

target_compile_options(trigger_core PRIVATE -Wall -Wextra -Werror)

add_executable(shell src/main.c)
target_link_libraries(shell trigger_core)
target_compile_options(shell PRIVATE -Wall -Wextra -Werror)

enable_testing()

add_executable(test_input tests/test_input.c)
target_link_libraries(test_input trigger_core)
target_compile_options(test_input PRIVATE -Wall -Wextra -Werror)
add_test(NAME InputTests COMMAND test_input)

add_executable(test_builtins tests/test_builtins.c)
target_link_libraries(test_builtins trigger_core)
target_compile_options(test_builtins PRIVATE -Wall -Wextra -Werror)
add_test(NAME BuiltinsTests COMMAND test_builtins)

add_executable(test_execute tests/test_execute.c)
target_link_libraries(test_execute trigger_core)
target_compile_options(test_execute PRIVATE -Wall -Wextra -Werror)
add_test(NAME ExecuteTests COMMAND test_execute)

add_executable(test_pipeline tests/test_pipeline.c)
target_link_libraries(test_pipeline trigger_core)
target_compile_options(test_pipeline PRIVATE -Wall -Wextra -Werror)
add_test(NAME PipelineTests COMMAND test_pipeline)

add_executable(test_glob tests/test_glob.c)
target_link_libraries(test_glob trigger_core)
target_compile_options(test_glob PRIVATE -Wall -Wextra -Werror)
add_test(NAME GlobTests COMMAND test_glob)

add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_input test_builtins test_execute test_pipeline test_glob
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
