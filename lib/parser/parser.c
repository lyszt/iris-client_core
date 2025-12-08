#include "./parser.h"
#include "commands/commands.h"
#include <string.h>
#include <stdio.h>



void route_command(int argc, char *argv[]) {
    if(argc < 2) {
        help_commands();
    }    
    
}