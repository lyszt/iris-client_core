#include <stdlib.h>
#include <stdio.h>
#include "parser/parser.h"
#include "term/term.h"


int main(int argc, char *argv[]) {
    iris_printf(IRIS_LOG_INFO, "Welcome to IRIS!\n\n");
    route_command(argc, argv);
    return 0;
}