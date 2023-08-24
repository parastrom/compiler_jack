#ifndef REFAC_COMPILER_H
#define REFAC_COMPILER_H

#include "refac_parser.h"

typedef struct {
    Arena* arena;
    vector jack_files;
    vector jack_vm_files;
    size_t num_of_files;
    char* vm_filename;
    FILE* vm_ptr;
    SymbolTable* global_table;
} CompilerState;

CompilerState* init_compiler();
int compile(CompilerState* state);
#endif