// Helper function to find the root directory containing .iris
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <git2.h>
#include "utils.h"

int find_iris_root(char *root_path, size_t max_len) {
	char current_dir[PATH_MAX];
	char check_path[PATH_MAX];

	if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
		return 0;
	}

	       while (1) {
		       size_t dir_len = strlen(current_dir);
		       const char *iris_suffix = "/.iris";
		       size_t suffix_len = strlen(iris_suffix);
		       if (dir_len + suffix_len >= sizeof(check_path)) {
			       // Path would be too long, abort
			       return 0;
		       }
		       snprintf(check_path, sizeof(check_path), "%s%s", current_dir, iris_suffix);

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

	return 0;
}

int get_current_branch(char *buf, size_t sz) {
	git_libgit2_init();
	git_repository *repo = NULL;
	int ret = 0;
	if (git_repository_open_ext(&repo, ".", 0, NULL) == 0) {
		git_reference *head = NULL;
		if (git_repository_head(&head, repo) == 0) {
			const char *name = git_reference_shorthand(head);
			if (name) {
				strncpy(buf, name, sz - 1);
				buf[sz - 1] = '\0';
				ret = 1;
			}
			git_reference_free(head);
		}
		git_repository_free(repo);
	}
	git_libgit2_shutdown();
	return ret;
}

/* Ensures .iris/branches/<branch>/ exists and writes the path into out. */
int iris_branch_dir(const char *iris_root, const char *branch, char *out, size_t sz) {
	char tmp[PATH_MAX];

	snprintf(tmp, sizeof(tmp), "%s/.iris/branches", iris_root);
	mkdir(tmp, 0755);

	snprintf(tmp, sizeof(tmp), "%s/.iris/branches/%s", iris_root, branch);
	mkdir(tmp, 0755);

	if ((size_t)snprintf(out, sz, "%s", tmp) >= sz) return 0;
	return 1;
}
