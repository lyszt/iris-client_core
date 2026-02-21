#include "init.h"
#include "term/term.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils/iris_template_utils.h"
#include "utils/iris_template_types.h"
#include <git2.h>

void return_error() {
  iris_printf(IRIS_LOG_ERROR, "Failed to initialize IRIS project.");
  return;
}



void init(const char *project_name) {
    if (system("mkdir -p .iris") != 0) {
        return_error();
        return;
    }

    char *name = NULL;
    if (project_name && project_name[0] != '\0') {
        name = strdup(project_name);
    } else {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            const char *base = strrchr(cwd, '/');
            if (base && base[1])
                name = strdup(base + 1);
            else if (strcmp(cwd, "/") == 0)
                name = strdup("root");
            else
                name = strdup("project");
        } else {
            name = strdup("project");
        }
    }
    if (!name) {
        return_error();
        return;
    }

    struct iris_template tpl = {0};
    tpl.project_name = name;
    tpl.command_lines = calloc(1, sizeof(Command));
    if (!write_iris_template(".iris/.iris.macros", &tpl)) {
        free(tpl.project_name);
        free(tpl.command_lines);
        return_error();
        return;
    }

    iris_printf(IRIS_LOG_INFO, "Initialized IRIS project '%s' in the current directory.\n", name);
    write_iris_to_exclude();
    free(tpl.project_name);
    free(tpl.command_lines);
}

void write_iris_to_exclude() {
    git_libgit2_init();

    git_repository *repo = NULL;

    if (git_repository_open_ext(&repo, ".", 0, NULL) != 0) {
        git_libgit2_shutdown();
        return;
    }


    if (git_repository_open_ext(&repo, ".", 0, NULL) != 0)
        return; // Not a git repo — silently skip

    const char *git_path = git_repository_path(repo);

    char exclude_path[PATH_MAX];
    snprintf(exclude_path, sizeof(exclude_path), "%sinfo/exclude", git_path);

    FILE *f = fopen(exclude_path, "r");
    int found = 0;

    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\r\n")] = 0;
            if (strcmp(line, ".iris/") == 0) {
                found = 1;
                break;
            }
        }
        fclose(f);
    }

    if (!found) {
        f = fopen(exclude_path, "a");
        if (f) {
            fprintf(f, "\n.iris/\n");
            fclose(f);
        }
    }

    git_repository_free(repo);
    git_libgit2_shutdown();
}