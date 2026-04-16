#include "ignore.h"
#include "term/term.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

static const char *ignore_suffix = "/.iris/.iris.ignore";

void iris_ignore(int argc, char **argv) {
    if (argc < 1) {
        iris_printf(IRIS_LOG_ERROR, "Usage: iris ignore <file> [file ...]\n");
        return;
    }

    char iris_location[PATH_MAX];
    if (!find_iris_root(iris_location, PATH_MAX)) {
        iris_printf(IRIS_LOG_ERROR, "Fatal: Not in an Iris project (or any of the parent directories).\n");
        return;
    }

    char ignore_path[PATH_MAX];
    if (strlen(iris_location) + strlen(ignore_suffix) >= sizeof(ignore_path)) {
        iris_printf(IRIS_LOG_ERROR, "Fatal: path too long.\n");
        return;
    }
    snprintf(ignore_path, sizeof(ignore_path), "%s%s", iris_location, ignore_suffix);

    for (int i = 0; i < argc; i++) {
        if (!argv[i] || !argv[i][0]) continue;

        /* Check if already present */
        FILE *f = fopen(ignore_path, "r");
        if (f) {
            char line[PATH_MAX];
            int found = 0;
            while (fgets(line, sizeof(line), f)) {
                line[strcspn(line, "\r\n")] = 0;
                if (strcmp(line, argv[i]) == 0) { found = 1; break; }
            }
            fclose(f);
            if (found) {
                iris_printf(IRIS_LOG_WARN, "'%s' already ignored.\n", argv[i]);
                continue;
            }
        }

        f = fopen(ignore_path, "a");
        if (!f) {
            iris_printf(IRIS_LOG_ERROR, "Cannot write to %s\n", ignore_path);
            return;
        }
        fprintf(f, "%s\n", argv[i]);
        fclose(f);
        iris_printf(IRIS_LOG_INFO, "Ignoring '%s' from copush.\n", argv[i]);
    }
}
