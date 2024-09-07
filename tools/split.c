#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_FILENAME_LENGTH 256
#define MAX_LINE_LENGTH 1024

void print_usage(const char* program_name) {
    fprintf(stderr, "Usage: %s --input <input_file> --output-path <output_path>\n", program_name);
}

int main(int argc, char* argv[]) {
    char* input_file = NULL;
    char* output_path = NULL;

    // Parse command-line arguments
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc) {
            fprintf(stderr, "Error: Missing argument value\n");
            print_usage(argv[0]);
            return 1;
        }
        if (strcmp(argv[i], "--input") == 0) {
            input_file = argv[i + 1];
        } else if (strcmp(argv[i], "--output-path") == 0) {
            output_path = argv[i + 1];
        } else {
            fprintf(stderr, "Error: Unknown argument %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!input_file || !output_path) {
        fprintf(stderr, "Error: Missing required arguments\n");
        print_usage(argv[0]);
        return 1;
    }

    FILE* input = fopen(input_file, "r");
    if (!input) {
        fprintf(stderr, "Error: Unable to open input file %s\n", input_file);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    char current_filename[MAX_FILENAME_LENGTH] = "";
    FILE* current_output = NULL;

    while (fgets(line, sizeof(line), input)) {
        if (strncmp(line, "#line 1 ", 8) == 0) {
            // Close previous output file if open
            if (current_output) {
                fclose(current_output);
                current_output = NULL;
            }

            // Extract new filename
            sscanf(line, "#line 1 \"%[^\"]\"", current_filename);

            // Open new output file
            char output_filename[MAX_FILENAME_LENGTH * 2];
            snprintf(output_filename, sizeof(output_filename), "%s/%s", output_path, current_filename);
            current_output = fopen(output_filename, "w");
            if (!current_output) {
                fprintf(stderr, "Error: Unable to open output file %s\n", output_filename);
                fclose(input);
                return 1;
            }
            // printf("Writing file: %s\n", current_filename);
        } else if (strncmp(line, "#line 0", 7) == 0) {
            // Close current output file
            if (current_output) {
                fclose(current_output);
                current_output = NULL;
            }
        } else if (current_output) {
            // Write line to current output file
            fputs(line, current_output);
        }
    }

    // Close any open files
    if (current_output) {
        fclose(current_output);
    }
    fclose(input);

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
