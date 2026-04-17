#ifndef IRIS_UTILS_H
#define IRIS_UTILS_H

#include <stddef.h>

int find_iris_root(char *root_path, size_t max_len);
int get_current_branch(char *buf, size_t sz);
int iris_branch_dir(const char *iris_root, const char *branch, char *out, size_t sz);

#endif
