#include "refac_lexer.h"
#include "logger.h"
#include "error.h"
#include "token.h"
#include <string.h>


//Allocates memory for the buffer and reads the file into it - buffer is owned by caller
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

    char* buffer = malloc(length + 1 * sizeof(char));
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

Lexer* init_lexer(const char* input) {
    Lexer* lexer = malloc(sizeof(Lexer));
    if (lexer == NULL) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for lexer\n");
        return NULL;
    }

    lexer->input = input;
    lexer->position = 0;
    lexer->queue = init_ringbuffer();
    lexer->peeked_token = NULL;
    lexer->current_token = NULL;
    if (lexer->queue == NULL) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for lexer queue\n");
        free(lexer);
        return NULL;
    }

    return lexer;
}

//Allocates memory for the newly stripped string - caller owns the memory
char* strip(const char* str) {
    size_t len = strlen(str);
    char* p = calloc(len + 1, sizeof(char));

    if (p) {
        size_t i = 0;
        for (size_t j = 0; j < len; j++) {
            if (str[j] != '\r' && str[j] != '\n') {
                p[i++] = str[j];
            }
        }
        p[i] = '\0';
    }

    return p;
}






