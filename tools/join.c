#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PATH 1024
#define MAX_LINE 4096

void process_template(const char* template_path, const char* base_path, const char* output_path) {
    FILE *template_file, *output_file;
    char line[MAX_LINE];
    char file_path[MAX_PATH];

    template_file = fopen(template_path, "r");
    if (!template_file) {
        fprintf(stderr, "Error opening template file: %s\n", template_path);
        exit(1);
    }

    output_file = fopen(output_path, "w");
    if (!output_file) {
        fprintf(stderr, "Error opening output file: %s\n", output_path);
        fclose(template_file);
        exit(1);
    }

    while (fgets(line, sizeof(line), template_file)) {
        if (strncmp(line, "{{FILE:", 7) == 0) {
            char* end = strchr(line + 7, '}');
            if (end) {
                *end = '\0';
                snprintf(file_path, sizeof(file_path), "%s%s", base_path, line + 7);
                // printf("Appending file: %s\n", line + 7);
                
                FILE* include_file = fopen(file_path, "r");
                if (include_file) {
                    fprintf(output_file, "#line 1 \"%s\"\n", line + 7);
                    char include_line[MAX_LINE];
                    while (fgets(include_line, sizeof(include_line), include_file)) {
                        fputs(include_line, output_file);
                    }
                    fprintf(output_file, "#line 0\n");
                    fclose(include_file);
                } else {
                    fprintf(stderr, "Warning: Unable to open included file: %s\n", file_path);
                    fputs(line, output_file);
                }
            } else {
                fputs(line, output_file);
            }
        } else {
            fputs(line, output_file);
        }
    }

    fclose(template_file);
    fclose(output_file);
}

int main(int argc, char* argv[]) {
    char template_path[MAX_PATH] = "";
    char base_path[MAX_PATH] = "";
    char output_path[MAX_PATH] = "";

    for (int i = 1; i < argc; i += 2) {
        if (i + 1 < argc) {
            if (strcmp(argv[i], "--template") == 0) {
                strncpy(template_path, argv[i + 1], sizeof(template_path) - 1);
            } else if (strcmp(argv[i], "--path") == 0) {
                strncpy(base_path, argv[i + 1], sizeof(base_path) - 1);
            } else if (strcmp(argv[i], "--output") == 0) {
                strncpy(output_path, argv[i + 1], sizeof(output_path) - 1);
            }
        }
    }

    if (template_path[0] == '\0' || base_path[0] == '\0' || output_path[0] == '\0') {
        fprintf(stderr, "Usage: %s --template <template_file> --path <base_path> --output <output_file>\n", argv[0]);
        return 1;
    }

    process_template(template_path, base_path, output_path);
    // printf("OK\n");

    return 0;
}

/*
   compiled with:
   cc -ObjC join.c -I../engine -o join.osx -O3
   cc join.c       -I../engine -o join.linux -O3
   tcc join.c      -I..\engine
   cl  join.c      -I..\engine /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
   del *.o & del *.obj & del *.lib & del *.exp & del *.pdb
*/
