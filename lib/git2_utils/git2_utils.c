#include "git2_utils.h"
#include "term/term.h"
#include <stdio.h>
#include <string.h>
#include <git2.h>
#include <stdio.h>
#include "utils.h"
#include <limits.h>

int iris_git_commit_and_push(const char *repo_path, const char *message) {
    git_libgit2_init();
    git_repository *repo = NULL;
    git_index *index = NULL;
    git_oid tree_id, commit_id;
    git_tree *tree = NULL;
    git_signature *sig = NULL;
    

    if (git_repository_open_ext(&repo, repo_path, 0, NULL) < 0) {
        iris_printf(IRIS_LOG_ERROR, "Could not open git repo (starting from %s)\n", repo_path);
        goto cleanup;
    }

    git_repository_index(&index, repo);
    git_index_add_all(index, NULL, 0, NULL, NULL);
    git_index_remove_directory(index, ".iris", 0);

    /* Remove user-ignored paths from .iris/.iris.ignore */
    {
        char iris_root[PATH_MAX] = {0};
        char ignore_path[PATH_MAX];
        if (!find_iris_root(iris_root, sizeof(iris_root)))
            strncpy(iris_root, repo_path, sizeof(iris_root) - 1);
        snprintf(ignore_path, sizeof(ignore_path), "%s/.iris/.iris.ignore", iris_root);
        FILE *ig = fopen(ignore_path, "r");
        if (ig) {
            char line[PATH_MAX];
            while (fgets(line, sizeof(line), ig)) {
                line[strcspn(line, "\r\n")] = 0;
                if (!line[0]) continue;
                char *pat = line;
                git_strarray patterns = { &pat, 1 };
                git_index_remove_all(index, &patterns, NULL, NULL);
            }
            fclose(ig);
        }
    }

    git_index_write(index);
    git_index_write_tree(&tree_id, index);
    git_tree_lookup(&tree, repo, &tree_id);

    if (git_signature_default(&sig, repo) < 0) {
        iris_printf(IRIS_LOG_ERROR, "Git identity not set. Use 'git config --global user.name ...'\n");
        goto cleanup;
    }


    git_oid parent_id;
    git_commit *parent = NULL;
    if (git_reference_name_to_id(&parent_id, repo, "HEAD") == 0) {
        git_commit_lookup(&parent, repo, &parent_id);
    }

    const git_commit *parents[] = { parent };
    git_commit_create(&commit_id, repo, "HEAD", sig, sig, NULL, message, tree, (parent ? 1 : 0), parents);

    iris_printf(IRIS_LOG_INFO, "Committed: %.7s\n", git_oid_tostr_s(&commit_id));

    system("git push");

cleanup:
    if (sig) git_signature_free(sig);
    if (tree) git_tree_free(tree);
    if (parent) git_commit_free(parent);
    if (index) git_index_free(index);
    if (repo) git_repository_free(repo);
    git_libgit2_shutdown();
    return 0;
}