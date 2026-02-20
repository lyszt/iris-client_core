#include "commit.h"
#include "term/term.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <errno.h>
#include "git2_utils/git2_utils.h"


void commit_push() {
	char commit_msg[1024];
	iris_printf(IRIS_LOG_INFO, "WARNING: This will stage all changes, commit and push.\n");
	iris_printf(IRIS_LOG_INFO, "Commiting... Please enter your commit message:\n");
	if (fgets(commit_msg, sizeof(commit_msg), stdin) == NULL) {
		iris_printf(IRIS_LOG_ERROR, "Failed to read commit message.\n");
		return;
	}
	// Remove trailing newline
	size_t len = strlen(commit_msg);
	if (len > 0 && commit_msg[len - 1] == '\n') {
		commit_msg[len - 1] = '\0';
	}
	if (strlen(commit_msg) == 0) {
		iris_printf(IRIS_LOG_ERROR, "Your message can't be empty.\n");
		return;
	}
	// Use libgit2 for commit and push
	int ret = iris_git_commit_and_push(".", commit_msg);
	if (ret != 0) {
		iris_printf(IRIS_LOG_ERROR, "Git commit/push failed.\n");
	}
}