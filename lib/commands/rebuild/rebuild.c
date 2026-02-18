#include "rebuild.h"
#include "term/term.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


void rebuild() {
	iris_printf(IRIS_LOG_INFO, "Rebuilding IRIS...\n");
	const char *build_dir = "../../build";
	char cmd[4096];

	snprintf(cmd, sizeof(cmd), "rm -rf %s", build_dir);
	int ret = system(cmd);
	if (ret != 0) {
		iris_printf(IRIS_LOG_ERROR, "Failed to remove build directory.\n");
		return;
	}
	snprintf(cmd, sizeof(cmd), "cd ../.. && make build");
	ret = system(cmd);
	if (ret != 0) {
		iris_printf(IRIS_LOG_ERROR, "make build failed.\n");
	} else {
		iris_printf(IRIS_LOG_INFO, "Build completed successfully.\n");
	}
}