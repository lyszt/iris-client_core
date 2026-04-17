#include "commit.h"
#include "term/term.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "git2_utils/git2_utils.h"
#include "components/select.h"
#include "utils.h"

#define MAX_MSG    4096
#define SUBJECT_LIMIT 72

static void save_cache(const char *iris_root, const char *msg) {
	char path[PATH_MAX];
	snprintf(path, sizeof(path), "%s/.iris/commit_cache", iris_root);
	FILE *f = fopen(path, "w");
	if (f) { fputs(msg, f); fclose(f); }
}

static int load_cache(const char *iris_root, char *buf, size_t sz) {
	char path[PATH_MAX];
	snprintf(path, sizeof(path), "%s/.iris/commit_cache", iris_root);
	FILE *f = fopen(path, "r");
	if (!f) return 0;
	size_t n = fread(buf, 1, sz - 1, f);
	buf[n] = '\0';
	fclose(f);
	return n > 0;
}

static char *prompt_required(const char *label) {
	char *s;
	while (1) {
		s = readline(label);
		if (s && s[0]) return s;
		iris_printf(IRIS_LOG_ERROR, "This field is required.\n");
		free(s);
	}
}

void commit_push(int argc, char **argv) {
	int retry = 0;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--retry") == 0 || strcmp(argv[i], "-r") == 0)
			retry = 1;
	}

	char iris_root[PATH_MAX] = {0};
	int has_root = find_iris_root(iris_root, sizeof(iris_root));
	char final_msg[MAX_MSG] = {0};

	if (retry && has_root) {
		if (!load_cache(iris_root, final_msg, sizeof(final_msg)) || !final_msg[0]) {
			iris_printf(IRIS_LOG_ERROR, "No cached commit found. Run iris commit normally first.\n");
			return;
		}
		iris_printf(IRIS_LOG_INFO, "Retrying with cached commit message.\n");
	} else {
		if (!iris_git_has_changes(".")) {
			iris_printf(IRIS_LOG_WARN, "Nothing to commit — working tree is clean.\n");
			return;
		}

		iris_printf(IRIS_LOG_WARN, "This will stage all changes, commit and push.\n");

		/* commit type */
		iris_printf(IRIS_LOG_INFO, "Select the type of commit:");
		const char *options[] = {
			"feat", "fix", "chore", "refactor", "docs", "test", "perf", "style", "build",
			"ci", "revert", "wip", "merge", "release", "security", "deps", "infra", "ux",
			"i18n", "hotfix", NULL
		};
		int choice = summon_select(options);
		const char *type = (choice >= 0 && options[choice]) ? options[choice] : "chore";

		/* optional scope */
		char *scope_raw = readline("Scope (optional, e.g. auth, api): ");
		char scope_buf[128] = {0};
		if (scope_raw && scope_raw[0])
			snprintf(scope_buf, sizeof(scope_buf), "(%s)", scope_raw);
		free(scope_raw);

		/* short description, max 72 chars */
		char *subject = NULL;
		while (1) {
			subject = prompt_required("Short description (max 72 chars): ");
			int len = (int)strlen(subject);
			if (len <= SUBJECT_LIMIT) break;
			iris_printf(IRIS_LOG_WARN, "Too long (%d chars). Please shorten to %d.\n", len, SUBJECT_LIMIT);
			free(subject);
			subject = NULL;
		}

		/* optional body (blank line ends input) */
		iris_printf(IRIS_LOG_INFO, "Longer description (blank line to finish):\n");
		char body_buf[2048] = {0};
		while (1) {
			char *line = readline("  ");
			if (!line || line[0] == '\0') { free(line); break; }
			if (body_buf[0])
				strncat(body_buf, "\n", sizeof(body_buf) - strlen(body_buf) - 1);
			strncat(body_buf, line, sizeof(body_buf) - strlen(body_buf) - 1);
			free(line);
		}

		/* breaking change */
		char breaking_desc[512] = {0};
		char *brk = readline("Breaking change? (y/N): ");
		if (brk && (brk[0] == 'y' || brk[0] == 'Y')) {
			free(brk);
			char *desc = readline("Describe the breaking change: ");
			if (desc && desc[0])
				snprintf(breaking_desc, sizeof(breaking_desc), "%s", desc);
			free(desc);
		} else {
			free(brk);
		}

		/* issue references */
		char *issues_raw = readline("Issue references (e.g. #42, blank to skip): ");
		char issues_buf[256] = {0};
		if (issues_raw && issues_raw[0])
			snprintf(issues_buf, sizeof(issues_buf), "%s", issues_raw);
		free(issues_raw);

		/* Build conventional commit message */
		int pos = snprintf(final_msg, sizeof(final_msg), "%s%s: %s", type, scope_buf, subject);
		free(subject);

		if (body_buf[0])
			pos += snprintf(final_msg + pos, sizeof(final_msg) - pos, "\n\n%s", body_buf);

		if (breaking_desc[0])
			pos += snprintf(final_msg + pos, sizeof(final_msg) - pos, "\n\nBREAKING CHANGE: %s", breaking_desc);

		if (issues_buf[0])
			pos += snprintf(final_msg + pos, sizeof(final_msg) - (size_t)pos, "\nCloses %s", issues_buf);
	}

	if (has_root)
		save_cache(iris_root, final_msg);

	int ret = iris_git_commit_and_push(".", final_msg);
	if (ret != 0)
		iris_printf(IRIS_LOG_ERROR, "Git commit/push failed.\n");
}
