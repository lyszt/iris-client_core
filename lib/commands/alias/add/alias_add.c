#include "alias_add.h"
#include "term/term.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "utils.h"

void alias_add() {

    char iris_location[PATH_MAX]; 

    if (find_iris_root(iris_location, PATH_MAX)) {
        iris_printf(IRIS_LOG_INFO, "alias add called. Iris location found at: %s\n", iris_location);
    } else {
        iris_printf(IRIS_LOG_ERROR, "Fatal: Not in an Iris project (or any of the parent directories)\n");
    }
}