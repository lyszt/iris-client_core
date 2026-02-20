#include "alias_add.h"
#include "term/term.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "utils.h"

void alias_add(int argc, char **argv) {
    (void)argc;
    (void)argv;
    char iris_location[PATH_MAX]; 

    if (find_iris_root(iris_location, PATH_MAX)) {
        iris_printf(IRIS_LOG_INFO, "alias add called. Iris location found at: %s\n", iris_location);
        char macro_path[PATH_MAX];
        const char *macro_suffix = "/.iris.macros";
        size_t loc_len = strlen(iris_location);
        size_t suffix_len = strlen(macro_suffix);
        if (loc_len + suffix_len >= sizeof(macro_path)) {
            iris_printf(IRIS_LOG_ERROR, "Fatal: macro_path too long\n");
            return;
        }
        snprintf(macro_path, sizeof(macro_path), "%s%s", iris_location, macro_suffix);
        
        // Next step is to read .iris.macro

        
    } else {
        iris_printf(IRIS_LOG_ERROR, "Fatal: Not in an Iris project (or any of the parent directories)\n");
    }
}