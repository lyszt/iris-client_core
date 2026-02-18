#include "init.h"
#include "term/term.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void return_error() {
  iris_printf(IRIS_LOG_ERROR, "Failed to initialize IRIS project.");
  return;
}

void init() {
    if (system("mkdir -p .iris") != 0) {
        return_error();
        return;
    }

    FILE *macros = fopen(".iris/.iris.macros", "w");
    if (!macros) {
        return_error();
        return;
    }

    fprintf(macros, "# Iris Project Macros\n");
    fclose(macros);

    iris_printf(IRIS_LOG_INFO, "Initialized IRIS project in the current directory.");
}