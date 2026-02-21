#ifndef UTILS_IRIS_TEMPLATE_H
#define UTILS_IRIS_TEMPLATE_H

#include <stddef.h>
#include <stdio.h>
#include "utils/iris_template_types.h"

/* Write template to binary file; appends num_macros=0 so file has a macros section. */
int write_iris_template(const char *filepath, const iris_template *tpl);

/* Read template only (stops before macros section). */
int read_iris_template(const char *filepath, iris_template *tpl);

/* Get command lines for macro `name`. Returns 1 if found, 0 otherwise. *lines and *n set; caller frees *lines and each (*lines)[i]. */
int get_macro_commands(const char *filepath, const char *name, char ***lines, size_t *n);

/* Append a new macro to the file. Creates file with empty template if missing. Caller keeps ownership of lines[]. */
int append_macro(const char *filepath, const char *name, char **lines, size_t n);

/* Free macros array (and each macro's name and lines). */
void free_macros(iris_macro *macros, size_t num_macros);

#endif
