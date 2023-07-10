#include "logger.h"  // Include the header file

LogLevel current_log_level = LOG_LEVEL_INFO;  // Define current_log_level here

void log_message(LogLevel level, const char* format, ...) {
    if (level > current_log_level) {
        return;
    }

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);  // Log to stderr, or replace this with your own logging code
    va_end(args);
}

void log_error(Error error) {
    log_message(LOG_LEVEL_ERROR, "Error (%d) at %s:%d: %s\n", error.code, error.file, error.line, error.message);
}
