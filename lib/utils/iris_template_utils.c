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
    if (fread(*str, 1, len, f) != len) { free(*str); *str = NULL; return 0; }
    (*str)[len] = '\0';
    return 1;
}

// Write only the template part (project_name + ncmds + command list). Does not close f.
static int write_template_part(FILE *f, const struct iris_template *tpl) {
    if (!write_string(f, tpl->project_name)) return 0;
    size_t ncmds = 0;
    if (tpl->command_lines) {
        while (tpl->command_lines[ncmds].command_line) ncmds++;
    }
    if (fwrite(&ncmds, sizeof(size_t), 1, f) != 1) return 0;
    for (size_t i = 0; i < ncmds; ++i) {
        if (fwrite(&tpl->command_lines[i].id, sizeof(int), 1, f) != 1) return 0;
        if (!write_string(f, tpl->command_lines[i].command_line)) return 0;
    }
    return 1;
}

// Skip the template section (so f is positioned at num_macros). Frees read strings.
static int skip_template(FILE *f) {
    char *p = NULL;
    if (!read_string(f, &p)) return 0;
    free(p);
    size_t ncmds = 0;
    if (fread(&ncmds, sizeof(size_t), 1, f) != 1) return 0;
    for (size_t i = 0; i < ncmds; ++i) {
        int id;
        if (fread(&id, sizeof(int), 1, f) != 1) return 0;
        if (!read_string(f, &p)) return 0;
        free(p);
        p = NULL;
    }
    return 1;
}

int write_iris_template(const char *filepath, const struct iris_template *tpl) {
    FILE *f = fopen(filepath, "wb");
    if (!f) return 0;
    if (!write_template_part(f, tpl)) { fclose(f); return 0; }
    size_t zero = 0;
    if (fwrite(&zero, sizeof(size_t), 1, f) != 1) { fclose(f); return 0; }
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
    tpl->command_lines[ncmds].command_line = NULL;
    fclose(f);
    return 1;
}

int get_macro_commands(const char *filepath, const char *name, char ***lines, size_t *n) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return 0;
    if (!skip_template(f)) { fclose(f); return 0; }
    size_t num_macros = 0;
    if (fread(&num_macros, sizeof(size_t), 1, f) != 1) { fclose(f); return 0; }
    *lines = NULL;
    *n = 0;
    for (size_t m = 0; m < num_macros; m++) {
        char *mname = NULL;
        if (!read_string(f, &mname)) { fclose(f); return 0; }
        size_t nlines = 0;
        if (fread(&nlines, sizeof(size_t), 1, f) != 1) { free(mname); fclose(f); return 0; }
        if (mname && strcmp(mname, name) == 0) {
            char **out = malloc(nlines * sizeof(char *));
            if (!out) { free(mname); fclose(f); return 0; }
            for (size_t i = 0; i < nlines; i++) {
                if (!read_string(f, &out[i])) {
                    while (i > 0) free(out[--i]);
                    free(out);
                    free(mname);
                    fclose(f);
                    return 0;
                }
            }
            free(mname);
            fclose(f);
            *lines = out;
            *n = nlines;
            return 1;
        }
        free(mname);
        for (size_t i = 0; i < nlines; i++) {
            char *line = NULL;
            if (!read_string(f, &line)) { fclose(f); return 0; }
            free(line);
        }
    }
    fclose(f);
    return 0;
}

static int write_macros_section(FILE *f, const iris_macro *macros, size_t num_macros) {
    if (fwrite(&num_macros, sizeof(size_t), 1, f) != 1) return 0;
    for (size_t m = 0; m < num_macros; m++) {
        if (!write_string(f, macros[m].name)) return 0;
        if (fwrite(&macros[m].n, sizeof(size_t), 1, f) != 1) return 0;
        for (size_t i = 0; i < macros[m].n; i++)
            if (!write_string(f, macros[m].lines[i])) return 0;
    }
    return 1;
}

static int read_macros_section(FILE *f, iris_macro **macros_out, size_t *num_out) {
    size_t num_macros = 0;
    if (fread(&num_macros, sizeof(size_t), 1, f) != 1) {
        *num_out = 0;
        *macros_out = NULL;
        return 1;
    }
    if (num_macros == 0) {
        *num_out = 0;
        *macros_out = NULL;
        return 1;
    }
    iris_macro *macros = calloc(num_macros, sizeof(iris_macro));
    if (!macros) return 0;
    size_t m;
    for (m = 0; m < num_macros; m++) {
        if (!read_string(f, &macros[m].name)) goto fail;
        if (fread(&macros[m].n, sizeof(size_t), 1, f) != 1) goto fail;
        if (macros[m].n == 0) {
            macros[m].lines = NULL;
            continue;
        }
        macros[m].lines = malloc(macros[m].n * sizeof(char *));
        if (!macros[m].lines) goto fail;
        for (size_t i = 0; i < macros[m].n; i++) {
            if (!read_string(f, &macros[m].lines[i])) {
                while (i > 0) free(macros[m].lines[--i]);
                free(macros[m].lines);
                macros[m].lines = NULL;
                macros[m].n = 0;
                goto fail;
            }
        }
    }
    *macros_out = macros;
    *num_out = num_macros;
    return 1;
fail:
    free_macros(macros, m + 1);
    return 0;
}

void free_macros(iris_macro *macros, size_t num_macros) {
    if (!macros) return;
    for (size_t m = 0; m < num_macros; m++) {
        free(macros[m].name);
        for (size_t i = 0; i < macros[m].n; i++) free(macros[m].lines[i]);
        free(macros[m].lines);
    }
    free(macros);
}

/* Deep-copy one macro. Returns 1 on success, 0 on alloc failure (caller must free partial). */
static int copy_one_macro(const iris_macro *src, iris_macro *dst) {
    dst->name = strdup(src->name);
    if (!dst->name) return 0;
    dst->n = src->n;
    if (src->n == 0) {
        dst->lines = NULL;
        return 1;
    }
    dst->lines = malloc(src->n * sizeof(char *));
    if (!dst->lines) { free(dst->name); return 0; }
    for (size_t i = 0; i < src->n; i++) {
        dst->lines[i] = src->lines[i] ? strdup(src->lines[i]) : NULL;
        if (src->lines[i] && !dst->lines[i]) {
            while (i > 0) free(dst->lines[--i]);
            free(dst->lines);
            free(dst->name);
            return 0;
        }
    }
    return 1;
}

int append_macro(const char *filepath, const char *name, char **lines, size_t n) {
    struct iris_template tpl = {0};
    iris_macro *macros = NULL;
    size_t num_macros = 0;

    if (!read_iris_template(filepath, &tpl)) {
        struct iris_template empty = {0};
        empty.project_name = strdup(".");
        empty.command_lines = NULL;
        if (!write_iris_template(filepath, &empty)) {
            free(empty.project_name);
            return 0;
        }
        free(empty.project_name);
        if (!read_iris_template(filepath, &tpl))
            return 0;
    }

    FILE *f = fopen(filepath, "rb");
    if (!f) return 0;
    if (!skip_template(f)) { fclose(f); return 0; }
    if (!read_macros_section(f, &macros, &num_macros)) { fclose(f); return 0; }
    fclose(f);

    /* Replace semantics: drop any existing macro with the same name. */
    size_t keep_count = 0;
    for (size_t i = 0; i < num_macros; i++) {
        if (strcmp(macros[i].name, name) != 0)
            keep_count++;
    }
    size_t new_n = keep_count + 1;
    iris_macro *new_macros = malloc(new_n * sizeof(iris_macro));
    if (!new_macros) { free_macros(macros, num_macros); free(tpl.project_name); if (tpl.command_lines) free(tpl.command_lines); return 0; }
    size_t j = 0;
    for (size_t i = 0; i < num_macros; i++) {
        if (strcmp(macros[i].name, name) != 0) {
            if (!copy_one_macro(&macros[i], &new_macros[j])) {
                free_macros(new_macros, j);
                free_macros(macros, num_macros);
                free(tpl.project_name);
                if (tpl.command_lines) free(tpl.command_lines);
                return 0;
            }
            j++;
        }
    }
    free_macros(macros, num_macros);
    macros = NULL;

    new_macros[keep_count].name = strdup(name);
    if (!new_macros[keep_count].name) { free_macros(new_macros, keep_count); free(tpl.project_name); if (tpl.command_lines) free(tpl.command_lines); return 0; }
    new_macros[keep_count].n = n;
    new_macros[keep_count].lines = malloc(n * sizeof(char *));
    if (!new_macros[keep_count].lines) { free(new_macros[keep_count].name); free_macros(new_macros, keep_count); free(tpl.project_name); if (tpl.command_lines) free(tpl.command_lines); return 0; }
    for (size_t i = 0; i < n; i++) {
        new_macros[keep_count].lines[i] = lines[i] ? strdup(lines[i]) : NULL;
        if (lines[i] && !new_macros[keep_count].lines[i]) {
            while (i > 0) free(new_macros[keep_count].lines[--i]);
            free(new_macros[keep_count].lines);
            free(new_macros[keep_count].name);
            free_macros(new_macros, keep_count);
            free(tpl.project_name);
            if (tpl.command_lines) free(tpl.command_lines);
            return 0;
        }
    }

    f = fopen(filepath, "wb");
    if (!f) { free_macros(new_macros, new_n); free(tpl.project_name); if (tpl.command_lines) free(tpl.command_lines); return 0; }
    if (!write_template_part(f, &tpl)) { fclose(f); free_macros(new_macros, new_n); free(tpl.project_name); if (tpl.command_lines) free(tpl.command_lines); return 0; }
    if (!write_macros_section(f, new_macros, new_n)) { fclose(f); free_macros(new_macros, new_n); free(tpl.project_name); if (tpl.command_lines) free(tpl.command_lines); return 0; }
    fclose(f);

    free(tpl.project_name);
    if (tpl.command_lines) free(tpl.command_lines);
    free_macros(new_macros, new_n);
    return 1;
}
