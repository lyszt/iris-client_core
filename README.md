# Iris (pronounced "Eris")

Iris is a developer CLI that wraps your git workflow with a structured commit flow, project-local macros, and a Prolog-powered command router that supports chaining and negation.

It lives in your shell. You run `iris` instead of raw git for day-to-day work.

---

## What it does

**`iris init [name]`** — sets up an `.iris/` directory in the current folder, writes the macros file, and adds `.iris/` to git's local exclude so it never shows up in your repo.

**`iris commit` / `iris copush`** — interactive commit + push in one shot. Prompts you to pick a conventional commit type (`feat`, `fix`, `refactor`, etc.) from a menu, then asks for a message, formats it as `type: message`, stages everything, commits, and pushes. Works from anywhere inside the project.

**`iris ignore <file>`** — adds a file to `.iris/.iris.ignore` so it's skipped during `copush`. Per-project, not global.

**`iris alias add <name> do <cmd> do <cmd>`** — saves a named macro (sequence of shell commands) to `.iris/.iris.macros`.

**`iris alias run <name>`** / **`iris run <name>`** — runs a saved macro.

**`iris rebuild`** — rebuilds the iris binary itself using CMake.

**`iris root`** — prints the detected iris project root.

---

## Command chaining

The command router is written in Prolog. It supports `&&`, `||`, and `not` directly in the argument list:

```bash
iris init myapp && iris commit      # commit only if init succeeded
iris init || true                   # ignore failure
iris not init                       # succeed when init fails
```

---

## Macros

Macros are stored in `.iris/.iris.macros` at the project root:

```ini
[deploy]
make build
git push origin main
ssh prod "systemctl restart app"
```

**Add with `do` separator** (no quoting needed):
```bash
iris alias add deploy do make build do git push origin main
```

**Add with quotes** (for commands that contain semicolons or shell syntax):
```bash
iris alias add test "npm run lint" "npm test"
```

**Run:**
```bash
iris run deploy
```

---

## Install

```bash
make install
```

This builds the binary, copies it to `/usr/local/bin/iris`, and appends an alias to your `.bashrc` or `.zshrc` (detected automatically). Then:

```bash
source ~/.zshrc   # or ~/.bashrc
```

Dependencies (`cmake`, `gcc`, `libcurl`, `libssl`, etc.) are checked and installed automatically during build if missing.

---

## Build from source

```bash
make        # configure + build (Debug)
make CONFIG=Release
make JOBS=8
```

Requires: `cmake`, `gcc`/`g++`, `libcurl-dev`, `libssl-dev`, `pkg-config`.  
Optional: SWI-Prolog (`libswipl-dev`) for the Prolog router — falls back to a C router if not present.

---

## Docs

- [User guide](docs/user-guide.md)
- [Architecture](docs/architecture.md)
- [Contributing](docs/contributing.md)
