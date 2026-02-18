#include "./parser.h"
#include "commands/commands.h"
#include "commands/init/init.h"
#include "commands/rebuild/rebuild.h"
#include "commands/commit/commit.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

void route_command(int argc, char *argv[]) {
  if (argc < 2) {
    help_commands();
    return;
  }

  char* command = argv[1];

  if(strcmp(command, "init") == 0) {
    init();
  }
  
  if(strcmp(command, "copush") == 0) {
    commit_push();
  }
  
}