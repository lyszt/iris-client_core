# Architecture

## Overview

Iris is a C executable that routes CLI arguments through an optional **Prolog router** (`lib/parser/iris_router.pl`). When SWI-Prolog is available, the router parses the command line into a list of goals (with `&&` / `||` / `not` and bracket groups); when not, a **C fallback** in `route_command()` handles a single command.

- **Entry**: `main()` in `src/main.c` resolves the Iris root (directory containing `.iris`), then calls `route_command(argc, argv, project_root)`.
- **Routing**: `lib/parser/parser.c` — if `IRIS_USE_PROLOG` is set, loads `iris_router.pl` and each command module’s `.pl` file, then calls `iris_goals/2` and executes the returned goal list. Otherwise it dispatches on `argv[1]` in C.
- **Commands**: Implementations live under `lib/commands/` (e.g. `init`, `commit`, `alias add`, `alias run`). Each can declare usage in a companion `.pl` file for the Prolog router.

## Prolog router

- **Input**: List of atoms from C `argv` (e.g. `[iris, init, myapp]` or `[iris, alias, add, deploy, [, git, push, ], &&, iris, commit]`).
- **Output**: List of `goal(Op, Command)` where `Op` is `and` or `or`, and `Command` is a term such as `init([myapp])`, `commit([])`, `alias_add(Args)`, `alias_run([name])`, or `not(Inner)`.
- **Stages**:
  1. Split on `&&` and `||` into segments; each segment has an connective (and/or).
  2. For each segment: drop program name, group tokens between `[` and `]` into single list elements, then parse with `parse_command/2`.
  3. `parse_command` uses explicit clauses for `not`, `alias add`, `alias run`, and otherwise `command_usage/3` from module `.pl` files to map the first token to a canonical command and consume parameters (`take_params/3`: `[]`, `[optional]`, or `[rest]`).

Commands are always returned as **compound terms** with one argument (a list of parameters), e.g. `init([])`, `init([myapp])`, `alias_add([deploy, [git, push]])`.

## C side (parser.c)

- **Loading**: After consulting `iris_router.pl`, C consults each `lib/commands/.../*.pl` (init, commit, rebuild, alias_add) so `command_usage/3` is populated.
- **Execution**: For each `goal(Op, Cmd)`, C evaluates `Cmd`: if compound with arity 1, it converts the argument list to `argc`/`argv` (with bracket groups flattened to one space-separated string per group), then dispatches by functor (`init`, `commit`, `rebuild`, `alias_add`, `alias_run`) and calls the corresponding C function. `not(Inner)` is handled by running `Inner` and inverting success/failure.
- **List conversion**: `pl_list_to_argv()` walks the Prolog list, copies atoms and flattens sublists into single strings; caller frees the resulting `argv`.

## Command modules

Each command can have:

- **C**: `lib/commands/<module>/<name>.c` and `.h` (e.g. `init.c`, `alias/add/alias_add.c`, `alias/run/alias_run.c`).
- **Prolog**: Optional `lib/commands/<module>/<name>.pl` declaring `command_usage(Canonical, Aliases, ParamSpec)`. Used by the router for single-token commands. Multi-token prefixes like `alias add` and `alias run` are handled by explicit clauses in `iris_router.pl`.

## Macros (.iris/macros)

- **Path**: `<project_root>/.iris/macros`. Project root is the directory containing `.iris` (found by `find_iris_root()`).
- **Format**: Human-readable text. Blocks are `[macro_name]` followed by one command per line; blocks are separated by blank lines. Empty lines within a block are skipped when running.
- **alias add**: Appends a new `[name]` block. Two forms: (1) use `do` as delimiter so each segment becomes one command line (tokens joined with space), e.g. `iris alias add test do echo hello world do git status`; (2) or pass one argument per command (user quotes), e.g. `iris alias add test "echo hello world" "git status"`. Bracket groups from the Prolog router still work if the user quotes them.
- **alias run**: Finds the `[name]` block and runs each non-empty line with `system()` in order.

## Binary template (.iris/.iris.macros)

`init` creates `.iris/.iris.macros` via `write_iris_template()` in a **binary** format (project name + list of `Command` structs). This is separate from the human-readable `.iris/macros` used by `alias add` / `alias run`.
