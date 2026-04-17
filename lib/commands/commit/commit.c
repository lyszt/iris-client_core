#include "commit.h"
#include "term/term.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "git2_utils/git2_utils.h"
#include "components/select.h"
#include "utils.h"

#define MAX_MSG    4096

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

static void write_template(FILE *f, const char *type) {
	fprintf(f,
		"%s: \n"
		"\n"
		"# Write your commit message above. Lines starting with '#' are ignored.\n"
		"#\n"
		"# First line format:  type(optional-scope): short description (max 72 chars)\n"
		"# Leave a blank line, then add a longer description below if needed.\n"
		"# Footer examples:\n"
		"#   BREAKING CHANGE: describe what breaks\n"
		"#   Closes #42\n"
		"#\n"
		"# Types: feat, fix, chore, refactor, docs, test, perf, style, build,\n"
		"#        ci, revert, wip, merge, release, security, deps, infra, ux,\n"
		"#        i18n, hotfix\n",
		type
	);
}

static int open_editor(const char *path) {
	const char *editor = getenv("VISUAL");
	if (!editor || !editor[0]) editor = getenv("EDITOR");

	if (editor && editor[0]) {
		char cmd[PATH_MAX + 128];
		snprintf(cmd, sizeof(cmd), "%s \"%s\"", editor, path);
		return system(cmd);
	}

	/* try common editors in order */
	const char *fallbacks[] = { "vim", "nano", "vi", NULL };
	for (int i = 0; fallbacks[i]; i++) {
		char which_cmd[64];
		snprintf(which_cmd, sizeof(which_cmd), "command -v %s >/dev/null 2>&1", fallbacks[i]);
		if (system(which_cmd) == 0) {
			char cmd[PATH_MAX + 128];
			snprintf(cmd, sizeof(cmd), "%s \"%s\"", fallbacks[i], path);
			return system(cmd);
		}
	}

	iris_printf(IRIS_LOG_ERROR, "No editor found. Set $VISUAL or $EDITOR.\n");
	return 1;
}

static int read_commit_msg(const char *path, char *buf, size_t sz) {
	FILE *f = fopen(path, "r");
	if (!f) return 0;

	size_t pos = 0;
	char line[1024];
	while (fgets(line, sizeof(line), f)) {
		if (line[0] == '#') continue;
		size_t len = strlen(line);
		if (pos + len >= sz - 1) break;
		memcpy(buf + pos, line, len);
		pos += len;
	}
	fclose(f);

	/* trim trailing whitespace */
	while (pos > 0 && (buf[pos-1] == '\n' || buf[pos-1] == '\r' || buf[pos-1] == ' '))
		pos--;
	buf[pos] = '\0';
	return pos > 0;
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

		/* write template to a temp file and open the user's editor */
		char tmppath[] = "/tmp/iris_commit_XXXXXX";
		int fd = mkstemp(tmppath);
		if (fd == -1) {
			iris_printf(IRIS_LOG_ERROR, "Failed to create temp file: %s\n", strerror(errno));
			return;
		}
		FILE *tmpf = fdopen(fd, "w");
		if (!tmpf) {
			close(fd);
			iris_printf(IRIS_LOG_ERROR, "Failed to open temp file.\n");
			return;
		}
		write_template(tmpf, type);
		fclose(tmpf);

		if (open_editor(tmppath) != 0) {
			iris_printf(IRIS_LOG_ERROR, "Editor exited with an error.\n");
			unlink(tmppath);
			return;
		}

		if (!read_commit_msg(tmppath, final_msg, sizeof(final_msg)) || !final_msg[0]) {
			iris_printf(IRIS_LOG_WARN, "Commit aborted — empty message.\n");
			unlink(tmppath);
			return;
		}
		unlink(tmppath);
	}

	if (has_root)
		save_cache(iris_root, final_msg);

	int ret = iris_git_commit_and_push(".", final_msg);
	if (ret != 0)
		iris_printf(IRIS_LOG_ERROR, "Git commit/push failed.\n");
}
