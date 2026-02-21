# Guide for AI agents (Cursor / IDE)

This file gives agents enough context to work on the Iris CLI codebase.

## What this project is

- **Iris CLI**: A C command-line tool. Commands are parsed by an embedded **Prolog router** (when SWI-Prolog is available) or a **C fallback**.
- **Features**: Command chaining (`&&`, `||`), negation (`not`), bracket groups for multi-word arguments, and **macros** (alias add/run) stored in `.iris/macros`.

## Key paths

| Path | Purpose |
|------|--------|
| `src/main.c` | Entry point; resolves Iris root, calls `route_command()`. |
| `src/utils.c`, `src/utils.h` | `find_iris_root()` — finds directory containing `.iris`. |
| `lib/parser/parser.c` | Loads Prolog router + command `.pl` files; runs goals; dispatches to C commands. |
| `lib/parser/iris_router.pl` | Prolog: parses argv into goals (`goal(Op, Command)`), uses `command_usage/3` from module `.pl` files. |
| `lib/commands/<cmd>/` | Command implementations (e.g. `init/`, `commit/`, `alias/add/`, `alias/run/`). |
| `lib/commands/<cmd>/*.pl` | Optional: declares `command_usage(Canonical, Aliases, ParamSpec)` for the router. |
| `.iris/macros` | Human-readable macro file (path is `<project_root>/.iris/macros`). |

## Build and run

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

Binary: `iris` at project root. Requires libcurl; optional libswipl-dev for Prolog router. Include dirs: `include`, `src`, `lib`, vendor libgit2.

## Conventions

- **Commands** get `(argc, argv)` from the parser (Prolog list converted to C). First token is often the “name” (e.g. macro name for `alias run`).
- **New command**: Add C under `lib/commands/`, optional `.pl` with `command_usage/3`, register in `parser.c` (`load_command_modules` if using `.pl`, and in `run_one_cmd()` for the functor), and in the C fallback branch if desired.
- **Macros**: Stored in `.iris/macros` (text: `[name]` blocks, one command per line). `alias add` appends; `alias run` finds `[name]` and runs each line with `system()`.
- **Prolog**: Single-token commands use `command_usage(Canonical, Aliases, ParamSpec)`. ParamSpec: `[]`, `[optional]`, or `[rest]`. Multi-token prefixes (`alias add`, `alias run`) are handled by explicit clauses in `iris_router.pl`.

## Documentation

- **User**: [docs/user-guide.md](docs/user-guide.md) — commands, chaining, macros, examples.
- **Architecture**: [docs/architecture.md](docs/architecture.md) — parser, router, macros, binary template.
- **Contributing**: [docs/contributing.md](docs/contributing.md) — build, layout, adding commands.
