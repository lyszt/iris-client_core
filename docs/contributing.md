# Contributing

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

The `iris` binary is written to the project root (or the path configured in CMake). Dependencies:

- **C11**, CMake 3.10+
- **libcurl** (required)
- **libgit2** (vendored under `vendor/libgit2-*`)
- **SWI-Prolog** (optional; e.g. `libswipl-dev`). If missing, the C-only router is used.

Include paths: `include`, `src`, `lib`, and libgit2’s `include`. Headers in `src/` (e.g. `utils.h`) are used by both `src/` and `lib/` code.

## Project layout

- **`src/`** — `main.c`, `utils.c` / `utils.h` (e.g. `find_iris_root`).
- **`lib/parser/`** — `parser.c`, `parser.h`, `iris_router.pl`; loads router and command `.pl` files.
- **`lib/commands/`** — One directory per command or subcommand (e.g. `init/`, `commit/`, `rebuild/`, `alias/add/`, `alias/run/`). Each can have `.c`, `.h`, and optionally `.pl`.
- **`lib/utils/`** — Shared utilities (e.g. `iris_template_utils.c`, `iris_template_types.h`).
- **`lib/term/`**, **`lib/git2_utils/`**, **`lib/network/`** — Term output, git2 helpers, network.
- **`docs/`** — User and architecture documentation.

## Adding a new command

1. **C implementation**  
   Add e.g. `lib/commands/foo/foo.c` and `foo.h`. Implement `void foo(int argc, char **argv)` (or no args if not needed).

2. **Prolog usage (single-token command)**  
   Add `lib/commands/foo/foo.pl` with:
   ```prolog
   :- multifile command_usage/3.
   command_usage(foo, [foo], []).   % or [optional], [rest], etc.
   ```
   Add the path to the `load_command_modules()` list in `lib/parser/parser.c`.

3. **Parser dispatch**  
   In `run_one_cmd()` in `lib/parser/parser.c`, handle the compound term `foo(Params)`: extract `argc`/`argv` (already done for the ref you get) and call `foo(argc, argv)`.

4. **Multi-token prefix (e.g. `alias run`)**  
   Add a clause in `iris_router.pl`, e.g. `parse_command([alias, run|Args], alias_run(Args)) :- !.` and dispatch in `parser.c`; no `.pl` needed unless you also want `command_usage` for something.

5. **C fallback**  
   In the `#else` branch of `route_command()` in `parser.c`, add a branch on `argv[1]` (and optionally `argv[2]`) to call your command when Prolog is not used.

## Conventions

- Commands receive parameters as a list from Prolog; C converts them to `argc`/`argv`. Use the first argument as the “name” or primary option when relevant (e.g. macro name for `alias run`).
- Macro file for alias add/run is `.iris/macros` (text). The binary file `.iris/.iris.macros` is written by `init` and uses the template utils; do not mix the two formats.
- Logging: use `iris_printf(IRIS_LOG_*)` from `term/term.h` for user-facing messages.
