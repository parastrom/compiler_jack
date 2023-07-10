#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>  
#include <stdio.h>   
#include <stdlib.h>
#include "error.h"

typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevel;

extern LogLevel current_log_level;  // Declare as extern here, define in logger.c

void log_message(LogLevel level, const char* format, ...);
void log_error(ErrorCode code, const char* file, int line, char* msg, ...);

#endif  // LOGGER_H
