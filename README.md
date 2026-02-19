# Trigger

## Overview
This project aims to create a clean, efficient shell. The current README was written quickly, but the code itself will be written without AI assistance - exception being repetitive tasks like folder organization and testing. I plan to build the shell based on solid fundamentals and personal expertise.

## Building

```bash
cmake -B cmake-build-debug -S .
cmake --build cmake-build-debug
```

## Running

```bash
./cmake-build-debug/shell
```

## Testing

The project includes comprehensive unit tests for all modules. To run the tests:

```bash
# Quick way
./run_tests.sh

# Or manually
cd cmake-build-debug
ctest --verbose
```

All tests use a custom lightweight testing framework that provides colored output and clear assertion messages.

## Goals
- Develop a functional, well‑structured shell.
- Ensure the shell is performant and easy to use.
- Continuously improve the project as I learn more.
- Maintain high code quality with unit tests.

## Inspiration
I’ll start by adapting the simple shell tutorial from Brennan Baker:

<https://brennan.io/2015/01/16/write-a-shell-in-c/>

This will serve as a solid foundation, which I’ll later expand and customize. The resulting shell, named **Trigger**, will combine the tutorial’s core ideas with additional features and optimizations.

Feel free to explore the repository, submit issues, or contribute improvements.
