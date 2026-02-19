// Helper function to find the root directory containing .iris
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include "utils.h"

int find_iris_root(char *root_path, size_t max_len) {
	char current_dir[PATH_MAX];
	char check_path[PATH_MAX];

	if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
		return 0;
	}

	while (1) {
		snprintf(check_path, sizeof(check_path), "%s/.iris", current_dir);

		if (access(check_path, F_OK) == 0) {
			strncpy(root_path, current_dir, max_len);
			return 1;
		}

		if (strcmp(current_dir, "/") == 0) {
			break;
		}

		char *last_slash = strrchr(current_dir, '/');
		if (last_slash != NULL) {
			if (last_slash == current_dir) {
				strcpy(current_dir, "/");
			} else {
				*last_slash = '\0';
			}
		} else {
			break;
		}
	}

	return 0; // .iris not found anywhere in the parent chain
}
