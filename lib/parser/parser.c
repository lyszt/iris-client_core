#include "./parser.h"
#include "commands/commands.h"
#include "commands/init/init.h"
#include "commands/rebuild/rebuild.h"
#include "commands/commit/commit.h"
#include "commands/alias/add/alias_add.h"
#include <limits.h>
#include <stdlib.h>
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

/* Consult each command module .pl so command_usage/3 is available. */
static int load_command_modules(const char *project_root) {
  const char *modules[] = {
    "lib/commands/init/init.pl",
    "lib/commands/commit/commit.pl",
    "lib/commands/rebuild/rebuild.pl",
    "lib/commands/alias/add/alias_add.pl",
  };
  for (size_t i = 0; i < sizeof(modules) / sizeof(modules[0]); i++) {
    char path[PATH_MAX];
    int n = snprintf(path, sizeof(path), "%s/%s", project_root, modules[i]);
    if (n < 0 || (size_t)n >= sizeof(path))
      return 0;
    term_t path_term = PL_new_term_ref();
    PL_put_atom_chars(path_term, path);
    predicate_t consult_pred = PL_predicate("consult", 1, "user");
    if (!PL_call_predicate(NULL, PL_Q_NODEBUG, consult_pred, path_term))
      return 0;
  }
  return 1;
}

/* Convert Prolog list of atoms (or sublists, flattened to one string per sublist) to argc/argv.
 * Caller frees *argv_out and each (*argv_out)[i]. Returns 0 on failure. */
static int pl_list_to_argv(term_t list_ref, int *argc_out, char ***argv_out) {
  term_t tail = PL_copy_term_ref(list_ref);
  term_t head = PL_new_term_ref();
  int cap = 32, n = 0;
  char **argv = (char **)malloc((size_t)cap * sizeof(char *));
  if (!argv) return 0;
  *argv_out = argv;
  *argc_out = 0;
  while (PL_get_list(tail, head, tail)) {
    char *s = NULL;
    if (PL_is_atom(head) && PL_get_atom_chars(head, &s)) {
      if (n >= cap) {
        cap *= 2;
        char **nargv = (char **)realloc(argv, (size_t)cap * sizeof(char *));
        if (!nargv) goto fail;
        argv = nargv;
        *argv_out = argv;
      }
      argv[n] = strdup(s);
      if (!argv[n]) goto fail;
      n++;
    } else if (PL_is_list(head)) {
      /* Bracket group: flatten to one space-separated string */
      term_t st = PL_copy_term_ref(head);
      term_t sh = PL_new_term_ref();
      size_t len = 0;
      int first = 1;
      while (PL_get_list(st, sh, st)) {
        char *a = NULL;
        if (PL_get_atom_chars(sh, &a)) {
          len += (first ? 0 : 1) + strlen(a);
          first = 0;
        }
      }
      if (PL_get_nil(st) && len > 0) {
        char *buf = (char *)malloc(len + 1);
        if (!buf) goto fail;
        buf[0] = '\0';
        st = PL_copy_term_ref(head);
        first = 1;
        while (PL_get_list(st, sh, st)) {
          char *a = NULL;
          if (PL_get_atom_chars(sh, &a)) {
            if (!first) strcat(buf, " ");
            strcat(buf, a);
            first = 0;
          }
        }
        if (n >= cap) {
          cap *= 2;
          char **nargv = (char **)realloc(argv, (size_t)cap * sizeof(char *));
          if (!nargv) { free(buf); goto fail; }
          argv = nargv;
          *argv_out = argv;
        }
        argv[n++] = buf;
      }
    }
  }
  if (!PL_get_nil(tail)) goto fail;
  *argc_out = n;
  return 1;
fail:
  while (n > 0) free(argv[--n]);
  free(argv);
  *argv_out = NULL;
  *argc_out = 0;
  return 0;
}

static void free_argv(int argc, char **argv) {
  if (!argv) return;
  for (int i = 0; i < argc; i++) free(argv[i]);
  free(argv);
}

/* Execute one command term: help (atom) or cmd(Params) compound. Returns 1 on success, 0 on failure. */
static int run_one_cmd(term_t cmd_ref, const char *project_root) {
  if (PL_is_atom(cmd_ref)) {
    char *name = NULL;
    if (!PL_get_atom_chars(cmd_ref, &name)) return 0;
    if (strcmp(name, "help") == 0) { help_commands(); return 1; }
    help_commands();
    return 0;
  }
  if (PL_is_compound(cmd_ref)) {
    atom_t name_atom;
    int arity;
    if (!PL_get_name_arity(cmd_ref, &name_atom, &arity)) return 0;
    const char *functor = PL_atom_chars(name_atom);
    if (functor && strcmp(functor, "not") == 0 && arity == 1) {
      term_t inner = PL_new_term_ref();
      PL_get_arg(1, cmd_ref, inner);
      return run_one_cmd(inner, project_root) ? 0 : 1;
    }
    if (functor && arity == 1) {
      term_t params_ref = PL_new_term_ref();
      PL_get_arg(1, cmd_ref, params_ref);
      int argc = 0;
      char **argv = NULL;
      if (!pl_list_to_argv(params_ref, &argc, &argv)) {
        help_commands();
        return 0;
      }
      int ok = 0;
      if (strcmp(functor, "init") == 0) {
        init(argc >= 1 ? argv[0] : ".");
        ok = 1;
      } else if (strcmp(functor, "commit") == 0) {
        commit_push();
        ok = 1;
      } else if (strcmp(functor, "rebuild") == 0) {
        rebuild(project_root);
        ok = 1;
      } else if (strcmp(functor, "alias_add") == 0) {
        alias_add(argc, argv);
        ok = 1;
      }
      free_argv(argc, argv);
      if (ok) return 1;
    }
  }
  help_commands();
  return 0;
}

/* Call iris_goals(Argv, GoalList) and run the goal chain (and/or/not). */
static void run_goals_via_prolog(int argc, char *argv[], const char *project_root) {
  term_t argv_list = PL_new_term_ref();
  term_t goal_list_ref = PL_new_term_ref();
  put_argv_list(argv_list, argc, argv);

  predicate_t iris_goals_pred = PL_predicate("iris_goals", 2, "user");
  term_t args = PL_new_term_refs(2);
  PL_put_term(args, argv_list);
  PL_put_variable(args + 1);

  if (!PL_call_predicate(NULL, PL_Q_NODEBUG | PL_Q_CATCH_EXCEPTION, iris_goals_pred, args)) {
    help_commands();
    return;
  }
  PL_put_term(goal_list_ref, args + 1);

  term_t head = PL_new_term_ref();
  term_t tail = PL_new_term_ref();
  term_t list_cursor = PL_new_term_ref();
  PL_put_term(list_cursor, goal_list_ref);

  int prev_success = 1; /* so first goal always runs */
  while (PL_get_list(list_cursor, head, tail)) {
    int goal_arity;
    atom_t goal_name;
    if (!PL_is_compound(head) || !PL_get_name_arity(head, &goal_name, &goal_arity) || goal_arity != 2) {
      PL_put_term(list_cursor, tail);
      continue;
    }
    term_t op_ref = PL_new_term_ref();
    term_t cmd_ref = PL_new_term_ref();
    PL_get_arg(1, head, op_ref);
    PL_get_arg(2, head, cmd_ref);

    char *op_str = NULL;
    (void)PL_get_atom_chars(op_ref, &op_str);
    int is_and = op_str && strcmp(op_str, "and") == 0;
    int is_or  = op_str && strcmp(op_str, "or") == 0;

    if (is_and && !prev_success) { PL_put_term(list_cursor, tail); continue; }
    if (is_or && prev_success)  { PL_put_term(list_cursor, tail); continue; }

    prev_success = run_one_cmd(cmd_ref, project_root);
    PL_put_term(list_cursor, tail);
  }
}

/* Legacy: resolve single command into buf (for callers that need one atom). */
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
    if (!load_router(project_root) || !load_command_modules(project_root)) {
      help_commands();
      return;
    }
  }

  run_goals_via_prolog(argc, argv, project_root);
#else
  /* C-only fallback when SWI-Prolog is not available */
  if (argc < 2) {
    help_commands();
    return;
  }

  char *command = argv[1];

  if (strcmp(command, "init") == 0) {
    init(argc >= 3 ? argv[2] : ".");
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
