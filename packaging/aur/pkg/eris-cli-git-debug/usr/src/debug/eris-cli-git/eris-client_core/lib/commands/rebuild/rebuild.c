#include "rebuild.h"
#include "term/term.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


void rebuild(const char *project_root) {
    eris_printf(ERIS_LOG_INFO, "Rebuilding ERIS with CMake...\n");

    if (chdir(project_root) != 0) {
        eris_printf(ERIS_LOG_ERROR, "Cannot find project root.\n");
        return;
    }

    int ret = system("cmake --build build --target eris");

    if (ret != 0) {
        eris_printf(ERIS_LOG_ERROR, "CMake build failed.\n");
    } else {
        eris_printf(ERIS_LOG_INFO, "Eris successfully rebuilt.\n");
    }
}