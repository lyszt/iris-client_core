# Iris CLI

Iris is a command-line client. Commands are routed via an embedded Prolog router that supports chaining, negation, and multi-token arguments.

## Documentation

- **[User guide](docs/user-guide.md)** — Commands, chaining, macros, bracket groups, examples.
- **[Architecture](docs/architecture.md)** — Parser, Prolog router, command modules, macro file format.
- **[Contributing](docs/contributing.md)** — Build, project layout, adding a new command.
- **[agents.md](agents.md)** — Guide for AI agents (Cursor / IDE).
- **[claude.md](claude.md)** — Guide for Claude.

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

The `iris` executable is written to the project root. If SWI-Prolog (e.g. `libswipl-dev`) is available, the Prolog router is used; otherwise a C-only fallback router is used.

## Usage

```text
iris <command> [arguments]
```

Run without a command to see the usage line.

### Commands

| Command | Aliases | Arguments | Description |
|---------|---------|-----------|-------------|
| `init` | — | `[PROJECT_NAME]` (optional) | Initialize an Iris project in the current directory. If `PROJECT_NAME` is omitted, `.` is used. |
| `commit` | `copush` | — | Stage all changes, commit (prompts for message), and push. |
| `rebuild` | — | — | Rebuild (project-specific). |
| `alias add` | — | `<name> do <cmd> do <cmd> ...` or `<name> "cmd1" "cmd2"` | Add a macro. Use `do` between commands (shell-friendly) or quote each command. |
| `alias run` | — | `<name>` | Run a macro by name (executes each stored command line via `system()`). |

### Chaining and negation (Prolog router)

When the Prolog router is enabled:

- **`&&`** — Run the next command only if the previous one succeeded.
- **`||`** — Run the next command only if the previous one failed (then stop).
- **`not`** — Invert success/failure of the following command (e.g. `not init` succeeds when `init` would fail).

Examples:

```bash
iris init myapp && iris commit
iris init || true
iris not init
```

### Macros (alias add / alias run)

Macros are stored in `.iris/macros` at the project root. The file is human-readable:

```ini
[macro_name]
command line one
command line two

[another]
single command
```

- **Add (shell-friendly):** `iris alias add <name> do <cmd> do <cmd> ...` — Use `do` between commands so you don’t need quoting. Example: `iris alias add test do echo hello world do git status`.
- **Add (quoted):** `iris alias add <name> "cmd1" "cmd2"` — One quoted string per command.
- **Run**: `iris alias run <name>` — Runs the macro (each line is executed with `system()` in order).

**Loops:** If the *shell* runs a loop that calls iris, keep the whole `iris` command on one line — the `do` inside the args is just a word: `for x in a b c; do iris alias add "$x" do echo "$x" do git status; done`. If a macro *contains* a shell loop (or any line with `;`), use the **quoted** form so the outer shell doesn’t split on semicolons: `iris alias add test "for x in 1 2 3; do echo \$x; done" "git status"`. See [User guide](docs/user-guide.md#macros-with-loops) for details.

### Bracket groups (Prolog router)

In Prolog mode you can group tokens with `[` and `]`; in bash/zsh you must quote the group. Prefer the `do` or quoted form for `alias add`.

## Project layout

- **`src/`** — `main.c`, `utils.c` / `utils.h` (e.g. `find_iris_root`).
- **`lib/parser/`** — Command routing: `iris_router.pl` (Prolog) and `parser.c` (C interface). Each command module can declare usage in a `.pl` file (e.g. `lib/commands/init/init.pl`).
- **`lib/commands/`** — Command implementations (init, commit, rebuild, alias add, alias run, etc.).
- **`docs/`** — [Documentation index](docs/README.md) (user guide, architecture, contributing).
