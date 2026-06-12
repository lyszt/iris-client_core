#ifndef ERIS_TERM_H
#define ERIS_TERM_H

#include <stdarg.h>
#include <stdbool.h>

typedef enum {
  ERIS_LOG_INFO,
  ERIS_LOG_WARN,
  ERIS_LOG_ERROR,
  ERIS_LOG_CMD,
  ERIS_LOG_DEBUG
} eris_log_level_t;

void eris_printf(eris_log_level_t level, const char *fmt, ...);

#endif
