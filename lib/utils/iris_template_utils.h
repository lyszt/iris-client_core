#ifndef UTILS_IRIS_TEMPLATE_H
#define UTILS_IRIS_TEMPLATE_H


#include <stdio.h>
#include "utils/iris_template_types.h"

// Write an iris_template struct to a binary file
int write_iris_template(const char *filepath, const iris_template *tpl);

// Read an iris_template struct from a binary file
int read_iris_template(const char *filepath, iris_template *tpl);

#endif
