#include <stdio.h>
#include "refac_compiler.h"

#ifndef JACK_FILES_DIR
    #define JACK_FILES_DIR "./jack_files"  // Fallback if JACK_FILES_DIR is not defined in CMakeLists.txt
#endif

#define PATH_LEN_MAX 1024

int main() {
    const char* jackFilesDir = JACK_FILES_DIR;
    printf("JACK files directory: %s\n", jackFilesDir);

    CompilerState* compilerState = init_compiler();
    int res = compile(compilerState);
    return 0;
}
