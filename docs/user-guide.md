# User guide

## Running Eris

From the project root (or any directory under an Eris project):

```bash
eris <command> [arguments]
```

Run `eris` with no arguments to see the usage line. The executable must be run from a path where the Eris root can be found (current directory or a parent containing `.eris`).

## Commands

| Command | Description |
|---------|-------------|
| `init [PROJECT_NAME]` | Initialize an Eris project in the current directory. Optional project name (default `.`). |
| `commit` / `copush` | Stage all changes, prompt for a commit message, then commit and push (libgit2). |
| `rebuild` | Rebuild the Eris binary (runs CMake in the Eris source tree). |
| `alias add <name> [cmd1] [cmd2] ...` | Add a macro: first argument is the name, rest are command lines (use brackets for multi-word commands). |
| `alias run <name>` | Run the named macro (each stored line is executed with `system()`). |

## Chaining and negation (Prolog router)

When built with SWI-Prolog:

- **`&&`** — Next command runs only if the previous succeeded.
- **`||`** — Next command runs only if the previous failed; then parsing stops.
- **`not`** — Success/failure of the following command is inverted (e.g. `not init` succeeds when `init` would fail).

Examples:

```bash
eris init myapp && eris commit
eris init || true
eris not init
```

## Macros

Macros are stored in `.eris/macros` at the project root. You can edit this file by hand.

**Add a macro (use `do` between commands; works in bash and zsh without quoting):**

```bash
eris alias add test do echo hello world do git status
eris alias add deploy do git push origin main do eris commit
```

**Or quote each command:**

```bash
eris alias add test "echo hello world" "git status"
```

**Run a macro:**

```bash
eris alias run status
eris alias run deploy
```

**File format:** Each macro is a `[name]` block followed by one command per line. Example:

```ini
[status]
git status

[deploy]
git push origin main
eris commit
```

### Macros with loops

**Loop in the shell (calling eris):** Use `do` as usual. Keep the whole `eris` invocation on one logical line so the shell doesn’t treat the inner `do` as the loop keyword:

```bash
for x in a b c; do eris alias add "$x" do echo "$x" do git status; done
```

**Loop inside a macro (stored and run by eris):** If one of the macro’s lines is a shell loop or uses `;`, the outer shell would split on semicolons and break the `do` form. Use the **quoted** form so the whole loop is one argument:

```bash
eris alias add test "for x in 1 2 3; do echo \$x; done" "git status"
```

Then `eris alias run test` runs the loop in the subshell; the macro file contains:

```ini
[test]
for x in 1 2 3; do echo $x; done
git status
```

## Bracket groups

In Prolog mode you can group tokens with `[` and `]`; in bash/zsh you must quote the group. For `alias add`, prefer the `do` or quoted form so you don’t need to quote brackets.
