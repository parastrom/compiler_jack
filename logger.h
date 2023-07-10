#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>  // For va_list, va_start(), and va_end()
#include <stdio.h>   // For printf()
#include <stdlib.h>  // For exit()
#include "error.h"

typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevel;

extern LogLevel current_log_level;  // Declare as extern here, define in logger.c

void log_message(LogLevel level, const char* format, ...);  // Declare functions here
void log_error(Error error);

#endif  // LOGGER_H
