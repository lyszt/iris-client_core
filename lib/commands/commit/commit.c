#include "commit.h"
#include "term/term.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <errno.h>
#include "git2_utils/git2_utils.h"
#include "components/select.h"

void commit_push() {
	char commit_msg[1024];
	iris_printf(IRIS_LOG_WARN, "WARNING: This will stage all changes, commit and push.\n");
	iris_printf(IRIS_LOG_INFO, "Select the type of commit:");
	const char* options[] = {
		"feat", "fix", "chore", "refactor", "docs", "test", "perf", "style", "build", 
		"ci", "revert", "wip", "merge", "release", "security", "deps", "infra", "ux", 
		"i18n", "localization", "hotfix", NULL
	};
	int choice = summon_select(options);

	iris_printf(IRIS_LOG_INFO, "Commiting... Please enter your commit message:\n");
	if (fgets(commit_msg, sizeof(commit_msg), stdin) == NULL)
    return;

	if (strlen(commit_msg) == 0) {
		iris_printf(IRIS_LOG_ERROR, "Your message can't be empty.\n");
		return;
	}

	commit_msg[strcspn(commit_msg, "\n")] = '\0';

	if (strlen(commit_msg) == 0)
		return;

	char final_msg[1024];
	if (choice >= 0 && options[choice])
		snprintf(final_msg, sizeof(final_msg), "%s: %s", options[choice], commit_msg);
	else
		snprintf(final_msg, sizeof(final_msg), "%s", commit_msg);

	// Use libgit2 for commit and push
	int ret = iris_git_commit_and_push(".", final_msg);
	if (ret != 0) {
		iris_printf(IRIS_LOG_ERROR, "Git commit/push failed.\n");
	}
}