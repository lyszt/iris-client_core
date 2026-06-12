#include "branch.h"
#include "term/term.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void branch_cmd(int argc, char **argv) {
    if (argc < 1 || !argv[0] || !argv[0][0]) {
        iris_printf(IRIS_LOG_ERROR, "Usage: iris branch <name>\n");
        return;
    }
    const char *name = argv[0];
    char cmd[512];

    iris_printf(IRIS_LOG_INFO, "Creating branch: %s\n", name);
    snprintf(cmd, sizeof(cmd), "git checkout -b %s", name);
    if (system(cmd) != 0) {
        iris_printf(IRIS_LOG_ERROR, "git checkout -b %s failed.\n", name);
        return;
    }

    iris_printf(IRIS_LOG_INFO, "Pushing upstream...\n");
    snprintf(cmd, sizeof(cmd), "git push --set-upstream origin %s", name);
    if (system(cmd) != 0) {
        iris_printf(IRIS_LOG_ERROR, "git push --set-upstream origin %s failed.\n", name);
        return;
    }

    iris_printf(IRIS_LOG_INFO, "Branch %s created and pushed.\n", name);
}
