# Guide for Claude

Context for working on the Iris CLI codebase.

## Project summary

Iris is a **C CLI** whose command line is parsed by an **embedded Prolog router** (SWI-Prolog) when available, otherwise by a **C fallback**. It supports:

- **Commands**: `init`, `commit`/`copush`, `rebuild`, `alias add`, `alias run`.
- **Chaining**: `&&` (then), `||` (else), `not` (invert success/failure).
- **Bracket groups**: `[ a b c ]` → one argument (e.g. multi-word commands for macros).
- **Macros**: Stored in `.iris/macros` (text file). `alias add <name> [cmd1] [cmd2] ...` appends a block; `alias run <name>` runs each line with `system()`.

## Code flow

1. **main** (`src/main.c`): Resolves Iris root (directory containing `.iris` via `find_iris_root()`), then calls `route_command(argc, argv, project_root)`.
2. **route_command** (`lib/parser/parser.c`): If Prolog is enabled, loads `iris_router.pl` and `lib/commands/*/.../*.pl`, calls `iris_goals(Argv, GoalList)`, then for each `goal(Op, Cmd)` runs the command (and/or/not logic). If Prolog is disabled, dispatches on `argv[1]` in C.
3. **Prolog** (`lib/parser/iris_router.pl`): Splits argv on `&&`/`||`, groups `[...]` tokens, then for each segment uses `parse_command/2` (and `command_usage/3` from module `.pl` files) to produce terms like `init([myapp])`, `alias_add(Args)`, `alias_run([name])`.
4. **run_one_cmd** (parser.c): For compound `cmd(Params)`, converts Params list to `argc`/`argv` (sublists flattened to one string), then calls the right C function (`init()`, `commit_push()`, `alias_add()`, `alias_run()`, etc.).

## Important files

- **Parser / router**: `lib/parser/parser.c`, `lib/parser/iris_router.pl`.
- **Commands**: `lib/commands/init/`, `lib/commands/commit/`, `lib/commands/rebuild/`, `lib/commands/alias/add/`, `lib/commands/alias/run/`.
- **Utils**: `src/utils.c` (`find_iris_root`), `lib/utils/` (e.g. template helpers). Include path includes `src/` so `utils.h` is found.
- **Macro file**: Path is `<project_root>/.iris/macros`; format is `[name]` blocks with one command per line.

## Adding or changing behavior

- **New single-token command**: Add C in `lib/commands/<name>/`, add `<name>.pl` with `command_usage(Canonical, [name], Spec)`, add path to `load_command_modules()` in parser.c, and in `run_one_cmd()` handle the functor and call your C function.
- **New multi-token command** (e.g. `alias run`): Add `parse_command([token1, token2|Args], functor(Args))` in `iris_router.pl`, and in `run_one_cmd()` handle `functor` and call the C implementation.
- **Macro format**: Only `.iris/macros` is used for alias add/run; `.iris/.iris.macros` is a binary template from `init` — do not mix.

## Docs

- [docs/user-guide.md](docs/user-guide.md) — Usage and examples.
- [docs/architecture.md](docs/architecture.md) — Parser, router, macros.
- [docs/contributing.md](docs/contributing.md) — Build and adding commands.
- [agents.md](agents.md) — Short agent-oriented overview.
