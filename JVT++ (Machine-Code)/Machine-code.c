#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "mips64.h"
#include "functions.h"

int main(int argc, char *argv[]) {
    // Check if a filename was provided as command-line argument
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        fprintf(stderr, "ERROR: No input file specified.\n");
        return 1;
    }

    // Open and read the source file
    char *source_file = open_source_file(argv[1]);
    if(source_file == NULL) {
        fprintf(stderr, "ERROR: File '%s' cannot be opened.\n", argv[1]);
        return 1;
    }

    // Convert the code (process and print converted code)
    code_convertion(source_file);

    // Clean up allocated memory
    free(source_file);

    return 0;
}