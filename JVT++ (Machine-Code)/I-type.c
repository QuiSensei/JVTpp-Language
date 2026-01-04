#include <string.h>
#include "mips64.h"

static const I_type i_type[] = {
    {"DADDIU", "011001"},
    {"SB",     "101000"},  // Store Byte
    {"LB",     "100000"},  // Load Byte
    {"SW",     "101011"},  // Store Word (32-bit) - NEW
    {"LW",     "100011"},  // Load Word (32-bit) - NEW
    {"LA",     "001111"}   // Load Address (pseudo-instruction) - NEW
};

const size_t i_type_count = sizeof(i_type) / sizeof(i_type[0]);

const char *get_i_type_code(const char *name) {
    for (size_t i = 0; i < i_type_count; ++i) {
        if (strcmp(i_type[i].Name, name) == 0) {
            return i_type[i].Op_code;
        }
    }
    return NULL; // Not found
}