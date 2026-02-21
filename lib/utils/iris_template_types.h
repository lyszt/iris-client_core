#ifndef IRIS_TEMPLATE_TYPES_H
#define IRIS_TEMPLATE_TYPES_H

#include <stddef.h>

typedef struct command {
    int id;
    char* command_line;
} Command;

typedef struct iris_template {
    char* project_name;
    Command* command_lines;
} iris_template;

/* One named macro: name + list of command lines (for .iris.macros macros section) */
typedef struct iris_macro {
    char* name;
    char** lines;
    size_t n;
} iris_macro;

#endif