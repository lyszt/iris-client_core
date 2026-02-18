#include "commit.h"
#include "term/term.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


void commit_push() {
	char commit_msg[1024];
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
	// Prepare git command
	char cmd[2048];
	snprintf(cmd, sizeof(cmd), "git add . && git commit -m \"%s\" && git push", commit_msg);
	int ret = system(cmd);
	if (ret != 0) {
		iris_printf(IRIS_LOG_ERROR, "Git command failed.\n");
	}
}