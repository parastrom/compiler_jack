#ifndef SAFER_H
#define SAFER_H

#include <stdlib.h>
#include "logger.h"

/**
 * @brief Allocates memory and checks if the allocation was successful
 * 
 * @param size 
 * @return void* 
 */
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
    } else {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not free memory");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Hashes a string
 * 
 * @param str 
 * @param size 
 * @return unsigned int 
 */
unsigned int hash(const char* str, size_t size) {
    unsigned int hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % size;
}


#endif // SAFER_MEM_H