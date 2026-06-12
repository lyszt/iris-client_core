#ifndef ERIS_TEMPLATE_TYPES_H
#define ERIS_TEMPLATE_TYPES_H

#include <stddef.h>

typedef struct command {
    int id;
    char* command_line;
} Command;

typedef struct eris_template {
    char* project_name;
    Command* command_lines;
} eris_template;

/* One named macro: name + list of command lines (for .eris.macros macros section) */
typedef struct eris_macro {
    char* name;
    char** lines;
    size_t n;
} eris_macro;

#endif