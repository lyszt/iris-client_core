#include "root.h"
#include "term/term.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


void root(const char *project_root) {
    iris_printf(IRIS_LOG_INFO, "%s", project_root);
}