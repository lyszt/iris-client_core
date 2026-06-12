#include "rebase.h"
#include "term/term.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Detect the repo's default branch via origin/HEAD; fall back to "master". */
static void detect_base_branch(char *buf, size_t sz) {
    FILE *p = popen("git symbolic-ref --short refs/remotes/origin/HEAD 2>/dev/null", "r");
    if (p) {
        if (fgets(buf, (int)sz, p)) {
            buf[strcspn(buf, "\r\n")] = '\0';
            /* strip leading "origin/" if symbolic-ref returned the long form */
            if (strncmp(buf, "origin/", 7) == 0)
                memmove(buf, buf + 7, strlen(buf + 7) + 1);
        } else {
            buf[0] = '\0';
        }
        pclose(p);
    } else {
        buf[0] = '\0';
    }
    if (!buf[0])
        snprintf(buf, sz, "master");
}

void rebase_cmd(int argc, char **argv) {
    char cmd[512];

    /* Base branch to rebase onto (e.g. master / main). */
    char base[256];
    detect_base_branch(base, sizeof(base));

    /* Target branch: explicit arg, else current branch. */
    char target[256] = {0};
    if (argc >= 1 && argv[0] && argv[0][0]) {
        snprintf(target, sizeof(target), "%s", argv[0]);
    } else if (!get_current_branch(target, sizeof(target)) || !target[0]) {
        eris_printf(ERIS_LOG_ERROR, "Could not determine current branch. Usage: eris rebase [branch]\n");
        return;
    }

    if (strcmp(target, base) == 0) {
        eris_printf(ERIS_LOG_WARN, "Target branch '%s' is the base branch — nothing to rebase.\n", base);
        return;
    }

    eris_printf(ERIS_LOG_INFO, "Fetching origin...\n");
    if (system("git fetch origin") != 0) {
        eris_printf(ERIS_LOG_ERROR, "git fetch origin failed.\n");
        return;
    }

    eris_printf(ERIS_LOG_INFO, "Updating base branch: %s\n", base);
    snprintf(cmd, sizeof(cmd), "git checkout %s && git pull", base);
    if (system(cmd) != 0) {
        eris_printf(ERIS_LOG_ERROR, "Failed to update base branch '%s'.\n", base);
        return;
    }

    eris_printf(ERIS_LOG_INFO, "Checking out target: %s\n", target);
    snprintf(cmd, sizeof(cmd), "git checkout %s", target);
    if (system(cmd) != 0) {
        eris_printf(ERIS_LOG_ERROR, "git checkout %s failed.\n", target);
        return;
    }

    eris_printf(ERIS_LOG_INFO, "Rebasing %s onto %s...\n", target, base);
    snprintf(cmd, sizeof(cmd), "git rebase %s", base);
    if (system(cmd) != 0) {
        eris_printf(ERIS_LOG_ERROR, "Rebase hit conflicts. Resolve them, then run 'git rebase --continue'.\n");
        return;
    }

    eris_printf(ERIS_LOG_INFO, "Rebase complete: %s is now on top of %s.\n", target, base);
}
