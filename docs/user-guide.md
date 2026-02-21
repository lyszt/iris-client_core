# User guide

## Running Iris

From the project root (or any directory under an Iris project):

```bash
iris <command> [arguments]
```

Run `iris` with no arguments to see the usage line. The executable must be run from a path where the Iris root can be found (current directory or a parent containing `.iris`).

## Commands

| Command | Description |
|---------|-------------|
| `init [PROJECT_NAME]` | Initialize an Iris project in the current directory. Optional project name (default `.`). |
| `commit` / `copush` | Stage all changes, prompt for a commit message, then commit and push (libgit2). |
| `rebuild` | Rebuild the Iris binary (runs CMake in the Iris source tree). |
| `alias add <name> [cmd1] [cmd2] ...` | Add a macro: first argument is the name, rest are command lines (use brackets for multi-word commands). |
| `alias run <name>` | Run the named macro (each stored line is executed with `system()`). |

## Chaining and negation (Prolog router)

When built with SWI-Prolog:

- **`&&`** — Next command runs only if the previous succeeded.
- **`||`** — Next command runs only if the previous failed; then parsing stops.
- **`not`** — Success/failure of the following command is inverted (e.g. `not init` succeeds when `init` would fail).

Examples:

```bash
iris init myapp && iris commit
iris init || true
iris not init
```

## Macros

Macros are stored in `.iris/macros` at the project root. You can edit this file by hand.

**Add a macro (use `do` between commands; works in bash and zsh without quoting):**

```bash
iris alias add test do echo hello world do git status
iris alias add deploy do git push origin main do iris commit
```

**Or quote each command:**

```bash
iris alias add test "echo hello world" "git status"
```

**Run a macro:**

```bash
iris alias run status
iris alias run deploy
```

**File format:** Each macro is a `[name]` block followed by one command per line. Example:

```ini
[status]
git status

[deploy]
git push origin main
iris commit
```

### Macros with loops

**Loop in the shell (calling iris):** Use `do` as usual. Keep the whole `iris` invocation on one logical line so the shell doesn’t treat the inner `do` as the loop keyword:

```bash
for x in a b c; do iris alias add "$x" do echo "$x" do git status; done
```

**Loop inside a macro (stored and run by iris):** If one of the macro’s lines is a shell loop or uses `;`, the outer shell would split on semicolons and break the `do` form. Use the **quoted** form so the whole loop is one argument:

```bash
iris alias add test "for x in 1 2 3; do echo \$x; done" "git status"
```

Then `iris alias run test` runs the loop in the subshell; the macro file contains:

```ini
[test]
for x in 1 2 3; do echo $x; done
git status
```

## Bracket groups

In Prolog mode you can group tokens with `[` and `]`; in bash/zsh you must quote the group. For `alias add`, prefer the `do` or quoted form so you don’t need to quote brackets.
