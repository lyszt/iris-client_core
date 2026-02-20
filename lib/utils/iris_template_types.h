#ifndef IRIS_TEMPLATE_TYPES_H
#define IRIS_TEMPLATE_TYPES_H

typedef struct command {
    int id;
    char* command_line;
} Command;

typedef struct iris_template {
    char* project_name;
    Command* command_lines;
} iris_template;

#endif