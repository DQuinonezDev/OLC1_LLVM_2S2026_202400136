#include "fuente.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char **lines = NULL;
static int line_count = 0;
static const char *file_name = "";

int fuente_cargar(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file)
        return 0;

    file_name = path;
    char buffer[1024];
    int capacity = 0;
    while (fgets(buffer, sizeof buffer, file)) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (line_count == capacity) {
            capacity = capacity ? capacity * 2 : 32;
            lines = realloc(lines, (size_t)capacity * sizeof *lines);
            if (!lines) {
                fputs("No hay memoria disponible.\n", stderr);
                exit(EXIT_FAILURE);
            }
        }
        lines[line_count] = malloc(strlen(buffer) + 1);
        if (!lines[line_count]) {
            fputs("No hay memoria disponible.\n", stderr);
            exit(EXIT_FAILURE);
        }
        strcpy(lines[line_count], buffer);
        line_count++;
    }
    fclose(file);
    return 1;
}

const char *fuente_nombre(void)
{
    return file_name;
}

/* Imprime:
     3 | let s = n + f;
       |         ^^^^^                                          */
void fuente_subrayar(Ubicacion loc)
{
    if (loc.line < 1 || loc.line > line_count)
        return;

    const char *text = lines[loc.line - 1];
    int end = loc.end_line == loc.line ? loc.end_col : (int)strlen(text);
    if (end < loc.col)
        end = loc.col;

    fprintf(stderr, "  %4d | %s\n", loc.line, text);
    fprintf(stderr, "       | ");
    for (int i = 1; i < loc.col; i++)
        fputc(text[i - 1] == '\t' ? '\t' : ' ', stderr);
    for (int i = loc.col; i <= end; i++)
        fputc('^', stderr);
    fputc('\n', stderr);
}

void fuente_liberar(void)
{
    for (int i = 0; i < line_count; i++)
        free(lines[i]);
    free(lines);
    lines = NULL;
    line_count = 0;
}
