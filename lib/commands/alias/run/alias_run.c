#include "alias_run.h"
#include "term/term.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void alias_run() {
    char root_path[PATH_MAX];
    if (find_iris_root(root_path, sizeof(root_path))) {
        iris_printf(IRIS_LOG_INFO, "alias run called. Project root found at: %s\n", root_path);
        // TODO: Implement alias run functionality using root_path
    } else {
        iris_printf(IRIS_LOG_ERROR, "Fatal: Not in an Iris project (or any of the parent directories)\n");
    }
}
