#include "alias_add.h"
#include "term/term.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "utils.h"

void alias_add(const char* project_root) {

    if (find_iris_root(project_root, PATH_MAX)) {
        iris_printf(IRIS_LOG_INFO, "alias add called. Project root found at: %s\n", project_root);
        // TODO: Implement alias add functionality using root_path
    } else {
        iris_printf(IRIS_LOG_ERROR, "Fatal: Not in an Iris project (or any of the parent directories)\n");
    }
}