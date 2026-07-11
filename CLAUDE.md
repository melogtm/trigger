# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

Trigger is a small POSIX shell written in C (C11), originally adapted from Brennan
Baker's "Write a Shell in C" tutorial and built with CMake. It now supports quoting,
pipelines, I/O redirection, and glob expansion on top of the tutorial's core loop.

## Build, run, test

Build:
    cmake -B cmake-build-debug -S .
    cmake --build cmake-build-debug

Run the shell:
    ./cmake-build-debug/shell

Run the full test suite:
    ./run_tests.sh
    # equivalent to: cmake -B/--build, then `cd cmake-build-debug && ctest --output-on-failure`

Run a single test module (CTest test names: InputTests, BuiltinsTests, ExecuteTests,
PipelineTests, GlobTests):
    cd cmake-build-debug && ctest -R PipelineTests --output-on-failure

Or run a test binary directly, e.g.:
    ./cmake-build-debug/test_pipeline

Lint (also enforced in CI as a separate `lint` job, see below):
    clang-format --dry-run --Werror $(find src include tests -name '*.c' -o -name '*.h')
    cmake -B cmake-build-debug -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    clang-tidy -p cmake-build-debug -warnings-as-errors=* $(find src include -name '*.c')

CI (`.github/workflows/ci.yml`) runs on every PR and push to `main` with two jobs:
`build-and-test` (`./run_tests.sh`) and `lint` (clang-format + clang-tidy, both
warnings-as-errors). Formatting follows `.clang-format` (LLVM base, 4-space indent,
100 col limit); tidy checks are configured in `.clang-tidy`. A tag matching `v*.*.*`
triggers `.github/workflows/release.yml`, which builds a Release binary and attaches
it to a GitHub release.

## Architecture

Data flow: `src/main.c` runs `trigger_loop()`, a read → split → execute → free REPL
loop. Each iteration reads a line, tokenizes it via `trigger_split_line_ex()`
(which also produces a parallel `glob_eligible` array), and hands both to
`trigger_execute()`.

- **Input/lexer** (`src/input.c`, `src/utils/utils.c`): `trigger_read_line()` reads
  a raw line via `getline`. `trigger_split_line_ex()` delegates to
  `parse_line_with_quotes()` in `src/utils/utils.c`, a hand-written state-machine
  tokenizer (`ParserState`: NORMAL / IN_SINGLE_QUOTE / IN_DOUBLE_QUOTE / ESCAPED)
  that handles single/double/backslash quoting and grows token/array buffers
  dynamically. This is the most complex and previously-buggy part of the codebase —
  trace through `process_character()` in `utils.c` before touching quote-parsing
  behavior. On unclosed quotes it returns `NULL` rather than crashing; callers must
  handle that. Alongside each token, it fills a parallel `glob_eligible` int array:
  a token is glob-ineligible if any part of it came from inside quotes or from a
  backslash escape, so `echo "*.c"` and `echo \*.c` are never expanded even though
  `echo *.c` is. Downstream code must keep `tokens[i]` and `glob_eligible[i]` in
  lockstep — they're always the same length and freed/reallocated together.
- **Glob expansion** (`src/glob.c`): `expand_globs()` runs after tokenizing and
  before pipeline/operator parsing. It only expands tokens marked glob-eligible and
  containing `*`, `?`, or `[`, using libc `glob()` with `GLOB_NOCHECK` (an unmatched
  pattern is passed through literally, shell-style). It rebuilds both the argv and
  glob_eligible arrays since one input token can expand to N output tokens.
- **Executor** (`src/execute.c`): `trigger_execute()` expands globs first, then
  scans the (glob-eligible-aware) token stream for `|`, `<`, `>`, `>>`. If any are
  present as real operators (not inside quotes), it hands off to
  `trigger_parse_pipeline()` / `trigger_execute_pipeline()` in `src/pipeline.c`.
  Otherwise it checks `args[0]` against the builtins table directly and falls
  through to `trigger_launch()` (fork() + execvp() + waitpid()) for external
  commands. A bare command with no operators never touches pipeline.c.
- **Pipelines & redirection** (`src/pipeline.c`): `trigger_parse_pipeline()` splits
  the token stream on `|` into `PipelineStage` structs (`argv`, `infile`, `outfile`,
  `append`), consuming `<`/`>`/`>>` and their filename arguments out of each stage's
  argv. `trigger_execute_pipeline()` has a fast path for a single stage with no
  redirection (checks builtins, else `trigger_launch()`), a path for a single
  builtin stage *with* redirection (saves/restores fds 0/1 around the builtin call
  via `dup`/`dup2` since builtins run in-process), and a general N-stage path that
  forks one child per stage, wires up `pipe()` fds between adjacent stages, and
  waits on all children. In the N-stage path, builtins run inside the forked child
  and `exit()` immediately after — only the single-stage paths run a builtin
  in-process without forking.
- **Builtins** (`src/builtins.c`): dispatch is two parallel arrays kept in lockstep
  by hand — `builtin_str[]` (names: cd, help, exit, pwd, echo, export, unset) and
  `builtin_func[]` (matching function pointers), iterated together by index in
  `trigger_execute()`, `trigger_execute_pipeline()`, and `trigger_help()`. Adding a
  builtin means appending to *both* arrays in the same position — there's no
  struct/table enforcing this, so mismatches are easy to introduce.

Headers in `include/` mirror `src/` (`input.h`, `execute.h`, `builtins.h`,
`pipeline.h`, `glob_expand.h`), except `utils/utils.h`, which lives next to its
`.c` file in `src/utils/` rather than in `include/`.

Tests in `tests/` are per-module (`test_input.c`, `test_builtins.c`,
`test_execute.c`, `test_pipeline.c`, `test_glob.c`) and link directly against the
relevant `.c` files rather than a shared static library, using a small custom
assertion framework in `tests/test_framework.h` (no external test dependency).
