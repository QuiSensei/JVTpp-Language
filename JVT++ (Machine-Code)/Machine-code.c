#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "mips64.h"
#include "functions.h"

int main(int argc, char *argv[]) {
    // Check if a filename was provided as command-line argument
    if(argc < 2) {
        fprintf(stderr, "ERROR: %s <source_file>\n", argv[0]);
        fprintf(stderr, "ERROR: No input file specified.\n");
        return 1;  // ✓ NOW INSIDE the if block
    }

    // Open and read the source file
    char *source_file = open_source_file(argv[1]);
    if(source_file == NULL) {
        fprintf(stderr, "ERROR: File '%s' cannot be opened.\n", argv[1]);
        return 1;
    }

    // Convert the code (process and print converted code)
    char *converted_code = code_convertion(source_file);
    if(converted_code == NULL) {
        fprintf(stderr, "ERROR: Code conversion failed.\n");
        free(source_file);
        return 1;
    }

    // Optionally print converted code for debugging
    // printf("%s", converted_code);

    const char *output_filename = "assembly.asm";
    // char output_filename[128] = "";  // needed if dynamic naming is required

    compile_to_assemble(converted_code, output_filename);

    // Clean up allocated memory
    free(converted_code);
    free(source_file);

    printf("Assembly code successfully generated: %s\n", output_filename);
    
    return 0;
}