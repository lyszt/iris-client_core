#include "commands.h"
#include "term/term.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CLR_RESET "\x1b[0m"
#define CLR_BOLD "\x1b[1m"
#define CLR_GREEN "\x1b[32m"
#define CLR_YELLOW "\x1b[33m"
#define CLR_CYAN "\x1b[36m"

void help_commands(void) {
    iris_printf(IRIS_LOG_CMD, "usage: iris <command> [arguments]\n\n");
    iris_printf(IRIS_LOG_CMD, "Commands:\n");
    iris_printf(IRIS_LOG_CMD, "  init [PROJECT_NAME]     Initialize an Iris project (default: current folder name)\n");
    iris_printf(IRIS_LOG_CMD, "  commit, copush          Stage, commit (prompt for message), push\n");
    iris_printf(IRIS_LOG_CMD, "  rebuild                 Rebuild iris binary\n");
    iris_printf(IRIS_LOG_CMD, "  alias add <name> do <cmd> do <cmd> ...  Add macro (do between commands)\n");
    iris_printf(IRIS_LOG_CMD, "  alias add <name> \"cmd1\" \"cmd2\"        Or quote each command\n");
    iris_printf(IRIS_LOG_CMD, "  alias run <name>        Run a macro (shorthand: iris run <name>)\n\n");
    iris_printf(IRIS_LOG_CMD, "Chaining (Prolog router): && (then)  || (else)  not <cmd> (invert)\n");
    iris_printf(IRIS_LOG_CMD, "Example: iris init myapp && iris commit\n");
}