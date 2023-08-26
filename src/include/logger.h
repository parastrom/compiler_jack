#ifndef LOGGER_H
#define LOGGER_H

#define log_error_no_offset(phase, code, filename, line, format, ...) log_error_internal(phase, code, filename, line, SIZE_MAX, format, ##__VA_ARGS__)
#define log_error_with_offset(phase, code, filename, line, byte_offset, format, ...) log_error_internal(phase, code, filename, line, byte_offset, format, ##__VA_ARGS__)


#include <stdarg.h>  
#include <stdio.h>   
#include <stdlib.h>
#include "error.h"
#include "arena.h"

typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevel;

extern LogLevel current_log_level;
extern Arena* loggerArena;


void log_message(LogLevel level, ErrorCode code, const char* format, ...);
char* get_line_from_file(const char* filepath, size_t byte_offset, Arena* arena);
const char* get_filename_from_path(const char* filepath);
void close_log_file();
void initialize_logger_arena();
void destroy_logger_arena();
#endif  // LOGGER_H
