#include "safer.h"

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

unsigned int hash(const char* str, size_t size) {
    unsigned int hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % size;
}
