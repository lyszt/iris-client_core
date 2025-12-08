#include "init.h"
#include "term/term.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


void init() {
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {

    iris_printf(IRIS_LOG_INFO, "Initializing project at %s...\n", cwd);
  } else {
    iris_printf(IRIS_LOG_ERROR, "getcwd() error: %s\n", strerror(errno));
  }
}