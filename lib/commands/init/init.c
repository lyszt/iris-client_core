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
  int ret = system("mkdir -p .iris");
  if (ret != 0) {
    return_error();
    return;
  }
  FILE *macros = fopen(".iris/.iris.macros", "w");

  if (ret == 0)
    macros = fopen(".iris/.iris.macros", "w");
  if (ret != 0 || !macros) {
    if (macros)
      fclose(macros);
    return_error();
    return;
  }
  fclose(macros);
  iris_printf(IRIS_LOG_INFO, "Initialized IRIS project.");
}