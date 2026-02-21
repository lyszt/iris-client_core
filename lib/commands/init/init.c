#include "init.h"
#include "term/term.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils/iris_template_utils.h"
#include "utils/iris_template_types.h"

void return_error() {
  iris_printf(IRIS_LOG_ERROR, "Failed to initialize IRIS project.");
  return;
}



void init(const char *project_name) {
    if (system("mkdir -p .iris") != 0) {
        return_error();
        return;
    }

    char *name = NULL;
    if (project_name && project_name[0] != '\0') {
        name = strdup(project_name);
    } else {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            const char *base = strrchr(cwd, '/');
            if (base && base[1])
                name = strdup(base + 1);
            else if (strcmp(cwd, "/") == 0)
                name = strdup("root");
            else
                name = strdup("project");
        } else {
            name = strdup("project");
        }
    }
    if (!name) {
        return_error();
        return;
    }

    struct iris_template tpl = {0};
    tpl.project_name = name;
    tpl.command_lines = calloc(1, sizeof(Command));
    if (!write_iris_template(".iris/.iris.macros", &tpl)) {
        free(tpl.project_name);
        free(tpl.command_lines);
        return_error();
        return;
    }
    iris_printf(IRIS_LOG_INFO, "Initialized IRIS project '%s' in the current directory.\n", name);
    free(tpl.project_name);
    free(tpl.command_lines);
}