#include "alias_add.h"
#include "term/term.h"
#include "utils/iris_template_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "utils.h"

/*
 * Macros are stored in the binary file .iris/.iris.macros (same file init creates).
 *
 * Add (shell-friendly): use "do" between commands, or quote each command.
 *   iris alias add <name> do <cmd1 words> do <cmd2 words> ...
 *   iris alias add <name> "cmd1" "cmd2"
 */

#define MAX_CMD_LINE 4096

void alias_add(int argc, char **argv) {
    char iris_location[PATH_MAX];

    if (!find_iris_root(iris_location, PATH_MAX)) {
        iris_printf(IRIS_LOG_ERROR, "Fatal: Not in an Iris project (or any of the parent directories).\n");
        return;
    }
    int is_global = 0;
    if (argc >= 1 && argv[0] && strcmp(argv[0], "--global") == 0) {
        is_global = 1;
        argv++; argc--;
    }

    if (argc < 1) {
        iris_printf(IRIS_LOG_ERROR, "Usage: iris alias add [--global] <name> do <cmd1> do <cmd2> ...\n");
        iris_printf(IRIS_LOG_ERROR, "   or: iris alias add [--global] <name> \"cmd1\" \"cmd2\"\n");
        return;
    }
    const char *name = argv[0];
    if (!name || !name[0]) {
        iris_printf(IRIS_LOG_ERROR, "Macro name cannot be empty.\n");
        return;
    }

    /* Build list of command lines: either split on "do" or one arg per line */
    char *lines[256];
    int nlines = 0;
    int use_delim = 0;
    for (int i = 1; i < argc; i++)
        if (argv[i] && strcmp(argv[i], "do") == 0) { use_delim = 1; break; }

    if (use_delim) {
        /* Split on "do": each segment becomes one command line (tokens joined with space) */
        char seg[MAX_CMD_LINE];
        size_t seglen = 0;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "do") == 0) {
                if (seglen > 0 && nlines < (int)(sizeof(lines) / sizeof(lines[0]))) {
                    seg[seglen] = '\0';
                    lines[nlines++] = strdup(seg);
                }
                seglen = 0;
                continue;
            }
            size_t len = strlen(argv[i]);
            if (seglen + (seglen ? 1 : 0) + len >= sizeof(seg))
                continue; /* skip token if segment would overflow */
            if (seglen > 0)
                seg[seglen++] = ' ';
            memcpy(seg + seglen, argv[i], len + 1);
            seglen += len;
        }
        if (seglen > 0 && nlines < (int)(sizeof(lines) / sizeof(lines[0]))) {
            seg[seglen] = '\0';
            lines[nlines++] = strdup(seg);
        }
    } else {
        /* No "do": join all remaining arguments into one command line (so e.g. echo "hello world" is one command) */
        if (argc > 1 && nlines < (int)(sizeof(lines) / sizeof(lines[0]))) {
            char one[MAX_CMD_LINE];
            size_t olen = 0;
            for (int i = 1; i < argc; i++) {
                if (!argv[i] || !argv[i][0]) continue;
                if (olen > 0) one[olen++] = ' ';
                size_t len = strlen(argv[i]);
                if (olen + len >= sizeof(one)) break;
                memcpy(one + olen, argv[i], len + 1);
                olen += len;
            }
            if (olen > 0)
                lines[nlines++] = strdup(one);
        }
    }

    char macro_path[PATH_MAX];
    if (is_global)
        snprintf(macro_path, sizeof(macro_path), "%s/.iris/.iris.macros", iris_location);
    else if (!iris_macros_path(iris_location, macro_path, sizeof(macro_path))) {
        for (int i = 0; i < nlines; i++) free(lines[i]);
        iris_printf(IRIS_LOG_ERROR, "Fatal: path too long.\n");
        return;
    }

    if (append_macro(macro_path, name, lines, (size_t)nlines))
        iris_printf(IRIS_LOG_INFO, "Macro '%s' added (%d command(s)).\n", name, nlines);
    else
        iris_printf(IRIS_LOG_ERROR, "Cannot write to %s\n", macro_path);
    for (int i = 0; i < nlines; i++)
        free(lines[i]);
}