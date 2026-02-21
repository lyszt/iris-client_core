#include "alias_run.h"
#include "term/term.h"
#include "utils.h"
#include "utils/iris_template_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Macros are in the binary file .iris/.iris.macros (same file init creates). */
static const char *macro_suffix = "/.iris/.iris.macros";

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

    for (size_t i = 0; i < n; i++) {
        if (!lines[i] || !lines[i][0]) continue;
        fflush(stdout);
        int ret = system(lines[i]);
        fflush(stdout);
        if (ret != 0)
            err("iris: command exited %d: %s\n", ret, lines[i]);
    }

    for (size_t i = 0; i < n; i++) free(lines[i]);
    free(lines);
}
