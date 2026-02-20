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

    struct iris_template tpl = {0};
    tpl.project_name = strdup(project_name);
    tpl.command_lines = calloc(1, sizeof(Command));
    if (!write_iris_template(".iris/.iris.macros", &tpl)) {
        free(tpl.project_name);
        free(tpl.command_lines);
        return_error();
        return;
    }
    free(tpl.project_name);
    free(tpl.command_lines);
    iris_printf(IRIS_LOG_INFO, "Initialized IRIS project in the current directory.");
}