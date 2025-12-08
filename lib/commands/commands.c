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

void help_commands() {
    iris_printf(IRIS_LOG_CMD, "usage: %s %s %s\n", "<iris>", "<command>", "<flags>");
}