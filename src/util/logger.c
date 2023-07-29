#include "logger.h"
#include <time.h>

#ifndef LOG_FILE
#define LOG_FILE "./logfile.log" // Fallback if LOG_FILE is not defined in CMakeLists.txt
#endif

LogLevel current_log_level = LOG_LEVEL_DEBUG; // Define current_log_level here
static FILE *log_file = NULL;                 // This will hold our file pointer

void log_message(LogLevel level, const char *format, ...)
{
    if (level > current_log_level)
    {
        return;
    }

    // Open the log file if it is not already open
    if (log_file == NULL)
    {
        log_file = fopen(LOG_FILE, "a"); // Open for appending using the LOG_FILE macro
        if (log_file == NULL)
        {
            perror("Failed to open log file"); // If fopen fails, perror will print a useful error message
            return;
        }
    }

    // Get the current time and format it as a string
    char timestamp[20];
    time_t now = time(NULL);
    struct tm result;
    struct tm *local_time = localtime_r(&now, &result);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);

    // Write the timestamp before the log message
    fprintf(log_file, "%s ", timestamp);

    // Write the log message
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args); // Write to log_file instead of stderr
    va_end(args);

    fflush(log_file); // Make sure the message is written immediately
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

    log_message(LOG_LEVEL_ERROR, "Error (%d) at %s:%d: %s\n", error.code, error.file, error.line, error.msg);

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
