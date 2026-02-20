#include "iris_template_utils.h"
#include "utils/iris_template_types.h"
#include <stdlib.h>
#include <string.h>

// Helper to write a string (length + data)
static int write_string(FILE *f, const char *str) {
    size_t len = str ? strlen(str) : 0;
    if (fwrite(&len, sizeof(size_t), 1, f) != 1) return 0;
    if (len > 0 && fwrite(str, 1, len, f) != len) return 0;
    return 1;
}

// Helper to read a string (length + data)
static int read_string(FILE *f, char **str) {
    size_t len = 0;
    if (fread(&len, sizeof(size_t), 1, f) != 1) return 0;
    if (len == 0) {
        *str = NULL;
        return 1;
    }
    *str = malloc(len + 1);
    if (!*str) return 0;
    if (fread(*str, 1, len, f) != len) { free(*str); return 0; }
    (*str)[len] = '\0';
    return 1;
}

int write_iris_template(const char *filepath, const struct iris_template *tpl) {
    FILE *f = fopen(filepath, "wb");
    if (!f) return 0;
    if (!write_string(f, tpl->project_name)) { fclose(f); return 0; }
    size_t ncmds = 0;
    if (tpl->command_lines) {
        while (tpl->command_lines[ncmds].command_line) ncmds++;
    }
    if (fwrite(&ncmds, sizeof(size_t), 1, f) != 1) { fclose(f); return 0; }
    for (size_t i = 0; i < ncmds; ++i) {
        if (fwrite(&tpl->command_lines[i].id, sizeof(int), 1, f) != 1) { fclose(f); return 0; }
        if (!write_string(f, tpl->command_lines[i].command_line)) { fclose(f); return 0; }
    }
    fclose(f);
    return 1;
}

int read_iris_template(const char *filepath, struct iris_template *tpl) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return 0;
    if (!read_string(f, &tpl->project_name)) { fclose(f); return 0; }
    size_t ncmds = 0;
    if (fread(&ncmds, sizeof(size_t), 1, f) != 1) { fclose(f); return 0; }
    if (ncmds == 0) {
        tpl->command_lines = NULL;
        fclose(f);
        return 1;
    }
    tpl->command_lines = calloc(ncmds + 1, sizeof(Command));
    if (!tpl->command_lines) { fclose(f); return 0; }
    for (size_t i = 0; i < ncmds; ++i) {
        if (fread(&tpl->command_lines[i].id, sizeof(int), 1, f) != 1) { fclose(f); return 0; }
        if (!read_string(f, &tpl->command_lines[i].command_line)) { fclose(f); return 0; }
    }
    tpl->command_lines[ncmds].command_line = NULL; // NULL-terminate
    fclose(f);
    return 1;
}
