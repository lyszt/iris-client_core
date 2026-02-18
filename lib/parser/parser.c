#include "./parser.h"
#include "commands/commands.h"
#include "commands/init/init.h"
#include "commands/rebuild/rebuild.h"
#include "commands/commit/commit.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

#ifdef IRIS_USE_PROLOG
#include <SWI-Prolog.h>

static int pl_initialised = 0;

/* Build a Prolog list of atoms from argv[0..argc-1]. List is built in term ref L. */
static void put_argv_list(term_t L, int argc, char *argv[]) {
  term_t a = PL_new_term_ref();
  PL_put_nil(L);
  for (int i = argc - 1; i >= 0; i--) {
    PL_put_atom_chars(a, argv[i]);
    PL_cons_list(L, a, L);
  }
}

/* Load iris_router.pl from project_root/lib/parser/iris_router.pl */
static int load_router(const char *project_root) {
  char path[PATH_MAX];
  int n = snprintf(path, sizeof(path), "%s/lib/parser/iris_router.pl", project_root);
  if (n < 0 || (size_t)n >= sizeof(path))
    return 0;
  term_t path_term = PL_new_term_ref();
  PL_put_atom_chars(path_term, path);
  predicate_t consult_pred = PL_predicate("consult", 1, "user");
  return PL_call_predicate(NULL, PL_Q_NODEBUG, consult_pred, path_term);
}

/* Resolve command via Prolog; write canonical command atom into buf, max len. Returns 1 on success. */
static int resolve_command_via_prolog(int argc, char *argv[], char *buf, size_t bufsize) {
  term_t argv_list = PL_new_term_ref();
  put_argv_list(argv_list, argc, argv);

  predicate_t iris_cmd = PL_predicate("iris_command", 2, "user");
  term_t args = PL_new_term_refs(2);
  PL_put_term(args, argv_list);
  PL_put_variable(args + 1);

  if (!PL_call_predicate(NULL, PL_Q_NODEBUG | PL_Q_CATCH_EXCEPTION, iris_cmd, args)) {
    return 0;
  }
  char *cmd_atom = NULL;
  if (!PL_get_atom_chars(args + 1, &cmd_atom))
    return 0;
  (void)snprintf(buf, bufsize, "%s", cmd_atom);
  return 1;
}
#endif

void route_command(int argc, char *argv[], const char *project_root) {
#ifdef IRIS_USE_PROLOG
  if (!pl_initialised) {
    /* Initialise Prolog with minimal argv so it finds the executable but doesn't consume our args */
    char *av[] = { (char *)(argv && argv[0] ? argv[0] : "iris"), NULL };
    int ac = 1;
    if (!PL_initialise(ac, av)) {
      help_commands();
      return;
    }
    pl_initialised = 1;
    if (!load_router(project_root)) {
      help_commands();
      return;
    }
  }

  char command[32];
  if (!resolve_command_via_prolog(argc, argv, command, sizeof(command))) {
    help_commands();
    return;
  }

  if (strcmp(command, "help") == 0) {
    help_commands();
    return;
  }
  if (strcmp(command, "init") == 0) {
    init();
    return;
  }
  if (strcmp(command, "commit") == 0) {
    commit_push();
    return;
  }
  if (strcmp(command, "rebuild") == 0) {
    rebuild(project_root);
    return;
  }
  help_commands();
#else
  /* C-only fallback when SWI-Prolog is not available */
  if (argc < 2) {
    help_commands();
    return;
  }

  char *command = argv[1];

  if (strcmp(command, "init") == 0) {
    init();
    return;
  }
  if (strcmp(command, "copush") == 0 || strcmp(command, "commit") == 0) {
    commit_push();
    return;
  }
  if (strcmp(command, "rebuild") == 0) {
    rebuild(project_root);
    return;
  }
  help_commands();
#endif
}
