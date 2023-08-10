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

/**
 * Read the contents of the file and store them in a string buffer.
 *
 * @param filename The name of the file to be read.
 * @return A pointer to the allocated buffer containing the file contents, or NULL on failure.
 *         The buffer is owned by the caller and should be freed when no longer needed.
 */
char* read_file_into_string(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        log_error(ERROR_FILE_OPEN, __FILE__, __LINE__, "Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    if (length < 0) {
        log_error(ERROR_FILE_READ, __FILE__, __LINE__, "Failed to determine file size");
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);

    if (length == 0) {
        log_error(ERROR_FILE_READ, __FILE__, __LINE__, "File is empty");
        fclose(file);
        return NULL;
    }

    char* buffer = safer_malloc(length + 1 * sizeof(char));
    if (buffer == NULL) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Failed to allocate memory for file content");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, length, file);
    if (bytes_read != (size_t)length) {
        log_error(ERROR_FILE_READ, __FILE__, __LINE__, "Failed to read file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[length] = '\0';

    fclose(file);
    return buffer;
}