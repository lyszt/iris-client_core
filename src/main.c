#include <stdlib.h>
#include <stdio.h>
#include "parser/parser.h"


int main(int argc, char *argv[]) {
    printf("Welcome to IRIS!\n");
    route_command(argc, argv);
    return 0;
}