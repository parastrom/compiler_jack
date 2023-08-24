#include "logger.h"
#include <time.h>

#ifndef LOG_FILE
#define LOG_FILE "./logfile.log" // Fallback if LOG_FILE is not defined in CMakeLists.txt
#endif

LogLevel current_log_level = LOG_LEVEL_INFO; // Define current_log_level here
static FILE *log_file = NULL;                 // This will hold our file pointer

const char* log_level_to_string(LogLevel level) {
    switch(level) {
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_WARNING: return "WARNING";
        case LOG_LEVEL_INFO: return "INFO";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

const char* error_code_to_string(ErrorCode code) {
    switch(code) {
        case ERROR_NONE: return "NONE";
        case ERROR_INVALID_INPUT: return "INVALID_INPUT";
        case ERROR_INVALID_OUTPUT: return "INVALID_OUTPUT";
        case ERROR_MEMORY_ALLOCATION: return "MEMORY_ALLOCATION";
        case ERROR_OPEN_DIRECTORY: return "OPEN_DIRECTORY";
        case ERROR_BUFFER_FULL: return "BUFFER_FULL";
        case ERROR_BUFFER_EMPTY: return "BUFFER_EMPTY";
        case ERROR_NULL_POINTER: return "NULL_POINTER";
        case ERROR_FILE_READ: return "FILE_READ";
        case ERROR_FILE_OPEN: return "FILE_OPEN";
        case ERROR_JSON_PARSING: return "JSON_PARSING";
        case ERROR_FILE_WRITE: return "FILE_WRITE";
        case ERROR_FILE_CLOSE: return "FILE_CLOSE";
        case ERROR_OUT_OF_BOUNDS: return "OUT_OF_BOUNDS";
        case ERROR_LEXER_NEWLINE_IN_STRING: return "LEXER_NEWLINE_IN_STRING";
        case ERROR_LEXER_EOF_IN_STRING: return "LEXER_EOF_IN_STRING";
        case ERROR_LEXER_UNEXPECTED_EOF: return "LEXER_UNEXPECTED_EOF";
        case ERROR_LEXER_ILLEGAL_SYMBOL: return "LEXER_ILLEGAL_SYMBOL";
        case ERROR_PARSER_UNEXPECTED_TOKEN: return "PARSER_UNEXPECTED_TOKEN";
        case ERROR_SEMANTIC_UNDECLARED_SYMBOL: return "SEMANTIC_UNDECLARED_SYMBOL";
        case ERROR_SEMANTIC_REDECLARED_SYMBOL: return "SEMANTIC_REDECLARED_SYMBOL";
        case ERROR_SEMANTIC_INVALID_SUBROUTINE: return "SEMANTIC_INVALID_SUBROUTINE";
        case ERROR_SEMANTIC_INVALID_TYPE: return "SEMANTIC_INVALID_TYPE";
        case ERROR_SEMANTIC_INVALID_KIND: return "SEMANTIC_INVALID_KIND";
        case ERROR_SEMANTIC_INVALID_INDEX: return "SEMANTIC_INVALID_INDEX";
        case ERROR_SEMANTIC_INVALID_SCOPE: return "SEMANTIC_INVALID_SCOPE";
        case ERROR_SEMANTIC_INVALID_STATEMENT: return "SEMANTIC_INVALID_STATEMENT";
        case ERROR_SEMANTIC_INVALID_EXPRESSION: return "SEMANTIC_INVALID_EXPRESSION";
        case ERROR_SEMANTIC_INVALID_TERM: return "SEMANTIC_INVALID_TERM";
        case ERROR_SEMANTIC_INVALID_OPERATION: return "SEMANTIC_INVALID_OPERATION";
        case ERROR_SEMANTIC_INVALID_VAR: return "SEMANTIC_INVALID_VAR";
        case ERROR_SEMANTIC_INVALID_ARGUMENT: return "SEMANTIC_INVALID_ARGUMENT";
        case ERROR_UNKNOWN_NODE_TYPE: return "UNKNOWN_NODE_TYPE";
        case ERROR_JSON_STRUCTURE: return "JSON_STRUCTURE";
        default: return "UNKNOWN_ERROR";
    }
}

void log_message(LogLevel level, ErrorCode code, const char *format, ...)
{
    if (level > current_log_level)
    {
        return;
    }

    if (log_file == NULL)
    {
        log_file = fopen(LOG_FILE, "a");
        if (log_file == NULL)
        {
            perror("Failed to open log file");
            return;
        }
    }

    char timestamp[20];
    time_t now = time(NULL);
    struct tm result;
    struct tm *local_time = localtime_r(&now, &result);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);

    fprintf(log_file, "%s [%s] [%s] ", timestamp, log_level_to_string(level),
            error_code_to_string(code));

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fflush(log_file);
}


void log_error(ErrorCode code, const char *file, int line, char *format, ...)
{
    va_list args;
    va_start(args, format);

    char msg[256];
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);

    Error error;
    error.code = code;
    error.line = line;
    error.file = file;
    error.msg = msg;

    log_message(LOG_LEVEL_ERROR, code, "%s:%d > %s\n",
                error.file, error.line, error.msg);

    set_error(error);
}

void close_log_file()
{
    if (log_file != NULL)
    {
        fclose(log_file);
        log_file = NULL;
    }
}
