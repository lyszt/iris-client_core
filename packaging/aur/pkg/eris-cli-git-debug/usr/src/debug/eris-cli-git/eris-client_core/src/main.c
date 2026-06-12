#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include "parser/parser.h"
#include "term/term.h"

#define ERIS_ROUTER_SUFFIX "/lib/parser/eris_router.pl"

/* Find directory that contains lib/parser/eris_router.pl (so Prolog router can load). */
static int get_eris_root(char *root, size_t size) {
    char path[PATH_MAX];
    char try_path[PATH_MAX];

    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) return 0;

    path[len] = '\0';
    char *dir = dirname(path);
    if (strlen(dir) >= size) return 0;
    strncpy(root, dir, size - 1);
    root[size - 1] = '\0';

    snprintf(try_path, sizeof(try_path), "%s%s", root, ERIS_ROUTER_SUFFIX);
    if (access(try_path, F_OK) == 0) return 1;

    /* Try parent (e.g. binary in build/, lib in repo root) */
    char parent_buf[PATH_MAX];
    strncpy(parent_buf, root, sizeof(parent_buf) - 1);
    parent_buf[sizeof(parent_buf) - 1] = '\0';
    char *parent = dirname(parent_buf);
    if (parent && strlen(parent) > 0) {
        snprintf(try_path, sizeof(try_path), "%s%s", parent, ERIS_ROUTER_SUFFIX);
        if (access(try_path, F_OK) == 0 && strlen(parent) < size) {
            strncpy(root, parent, size - 1);
            root[size - 1] = '\0';
            return 1;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        eris_printf(ERIS_LOG_ERROR, "No command provided. Usage: eris <command>\n");
        return 1;
    }

    char project_root[PATH_MAX] = {0};
    
    if (!get_eris_root(project_root, sizeof(project_root))) {
        eris_printf(ERIS_LOG_ERROR, "Could not determine Eris root location.\n");
        return 1;
    }

    return route_command(argc, argv, project_root) ? 0 : 1;
}