#ifndef IRIS_TERM_H
#define IRIS_TERM_H

#include <stdarg.h>
#include <stdbool.h>

typedef enum {
  IRIS_LOG_INFO,
  IRIS_LOG_WARN,
  IRIS_LOG_ERROR,
  IRIS_LOG_CMD,
  IRIS_LOG_DEBUG
} iris_log_level_t;

void iris_printf(iris_log_level_t level, const char *fmt, ...);

#endif
