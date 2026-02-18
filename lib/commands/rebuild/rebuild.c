#include "rebuild.h"
#include "term/term.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


void rebuild(const char *project_root) {
    iris_printf(IRIS_LOG_INFO, "Rebuilding IRIS with CMake...\n");

    if (chdir(project_root) != 0) {
        iris_printf(IRIS_LOG_ERROR, "Cannot find project root.\n");
        return;
    }

    int ret = system("cmake --build build --target iris");

    if (ret != 0) {
        iris_printf(IRIS_LOG_ERROR, "CMake build failed.\n");
    } else {
        iris_printf(IRIS_LOG_INFO, "Iris successfully rebuilt.\n");
    }
}