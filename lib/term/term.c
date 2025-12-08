#include "term.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define CLR_RESET "\x1b[0m"
#define CLR_BOLD "\x1b[1m"
#define CLR_RED "\x1b[31m"
#define CLR_GREEN "\x1b[32m"
#define CLR_YELLOW "\x1b[33m"
#define CLR_CYAN "\x1b[36m"

static bool cached_colors = false;
static bool cached_colors_inited = false;

static bool iris_colors_enabled(void) {
    if (cached_colors_inited) return cached_colors;
    cached_colors_inited = true;
    if (getenv("NO_COLOR") || getenv("IRIS_NO_COLOR")) return false;
    return (cached_colors = isatty(fileno(stdout)));
}

void iris_printf(iris_log_level_t level, const char *fmt, ...) {
    FILE *out = stdout;
    const char *col = "";
    const char *reset = "";
    if (level == IRIS_LOG_ERROR) out = stderr;
    if (iris_colors_enabled()) {
        reset = CLR_RESET;
        switch (level) {
            case IRIS_LOG_ERROR: col = CLR_RED; break;
            case IRIS_LOG_WARN: col = CLR_YELLOW; break;
            case IRIS_LOG_INFO: col = CLR_GREEN; break;
            case IRIS_LOG_CMD: col = CLR_CYAN; break;
            case IRIS_LOG_DEBUG: col = CLR_BOLD; break;
            default: col = ""; break;
        }
    }
    if (col[0]) fprintf(out, "%s", col);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    va_end(ap);
    if (reset[0]) fprintf(out, "%s", reset);
}
