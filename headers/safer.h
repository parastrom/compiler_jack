#ifndef SAFER_H
#define SAFER_H

#include <stdlib.h>
#include "logger.h"


void* safer_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void safer_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}


#endif // SAFER_MEM_H