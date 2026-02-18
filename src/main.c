#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include "parser/parser.h"
#include "term/term.h"


static int get_iris_root(char *root, size_t size) {
    char path[PATH_MAX];
    
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) return 0;

    path[len] = '\0';
    char *dir = dirname(path);
    
    if (strlen(dir) >= size) return 0;

    strncpy(root, dir, size - 1);
    root[size - 1] = '\0';
    
    return 1;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        iris_printf(IRIS_LOG_ERROR, "No command provided. Usage: iris <command>\n");
        return 1;
    }

    char project_root[PATH_MAX] = {0};
    
    if (!get_iris_root(project_root, sizeof(project_root))) {
        iris_printf(IRIS_LOG_ERROR, "Could not determine Iris root location.\n");
        return 1;
    }

    route_command(argc, argv, project_root);
    
    return 0;
}