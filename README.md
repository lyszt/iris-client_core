# Iris CLI

Iris is a command-line client. Commands are routed via an embedded Prolog router that supports chaining, negation, and multi-token arguments.

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
| `alias add` | — | `[tokens...]` | Add an alias; tokens can be grouped in brackets (see below). |

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

### Bracket groups

Arguments can be grouped with `[` and `]` so that multiple tokens are passed as one logical argument (e.g. for aliases):

```bash
iris alias add [ git push origin main ]
```

The router treats everything between `[` and `]` as a single group.

## Project layout

- **`lib/parser/`** — Command routing: `iris_router.pl` (Prolog) and `parser.c` (C interface). Each command module can declare usage in a `.pl` file (e.g. `lib/commands/init/init.pl`).
- **`lib/commands/`** — Command implementations (init, commit, rebuild, alias add, etc.).
