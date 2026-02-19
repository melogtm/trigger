cmake_minimum_required(VERSION 3.28.3)
project(shell C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

set(SOURCES
        src/main.c
        src/input.c
        src/execute.c
        src/builtins.c
)

set(HEADERS
        include/input.h
        include/execute.h
        include/builtins.h
)

add_executable(shell ${SOURCES} ${HEADERS})
