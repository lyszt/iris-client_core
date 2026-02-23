#include "alias_run.h"
#include "term/term.h"
#include "utils.h"
#include "utils/iris_template_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

/* Macros are in the binary file .iris/.iris.macros (same file init creates). */
static const char *macro_suffix = "/.iris/.iris.macros";

#define MAX_CMD_LINE 4096

static void err(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fflush(stderr);
}

void alias_run(int argc, char **argv) {
    char root_path[PATH_MAX];
    if (!find_iris_root(root_path, sizeof(root_path))) {
        err("iris: Not in an Iris project (run 'iris init' first).\n");
        return;
    }
    if (argc < 1 || !argv[0] || !argv[0][0]) {
        err("iris: Usage: iris alias run <name>\n");
        return;
    }
    const char *name = argv[0];
    char macro_path[PATH_MAX];
    if (strlen(root_path) + strlen(macro_suffix) >= sizeof(macro_path)) {
        err("iris: path too long.\n");
        return;
    }
    snprintf(macro_path, sizeof(macro_path), "%s%s", root_path, macro_suffix);

    char **lines = NULL;
    size_t n = 0;
    if (!get_macro_commands(macro_path, name, &lines, &n)) {
        err("iris: Macro '%s' not found or has no commands (add with 'iris alias add %s ...').\n", name, name);
        return;
    }

    char exec_path[PATH_MAX];
    exec_path[0] = '\0';
#if defined(__linux__)
    {
        ssize_t len = readlink("/proc/self/exe", exec_path, sizeof(exec_path) - 1);
        if (len > 0) {
            exec_path[len] = '\0';
        }
    }
#endif

    for (size_t i = 0; i < n; i++) {
        if (!lines[i] || !lines[i][0]) continue;
        const char *run = lines[i];
        char buf[MAX_CMD_LINE];
        if (exec_path[0]) {
            if (strcmp(lines[i], "iris") == 0) {
                run = exec_path;
            } else if (strncmp(lines[i], "iris ", 5) == 0) {
                int need = snprintf(buf, sizeof(buf), "%s %s", exec_path, lines[i] + 5);
                if (need > 0 && (size_t)need < sizeof(buf))
                    run = buf;
            }
        }
        fflush(stdout);
        int ret = system(run);
        fflush(stdout);
        if (ret != 0)
            err("iris: command exited %d: %s\n", ret, lines[i]);
    }

    for (size_t i = 0; i < n; i++) free(lines[i]);
    free(lines);
}
