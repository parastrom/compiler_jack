#include "logger.h"
#include <string.h>
#include <time.h>



#ifndef LOG_FILE
#define LOG_FILE "./logfile.log" // Fallback if LOG_FILE is not defined in CMakeLists.txt
#endif

const int multiplier  = 2;
LogLevel current_log_level = LOG_LEVEL_INFO;
Arena* loggerArena = NULL;
static FILE *log_file = NULL;

void initialize_logger_arena() {
    if (!loggerArena) {
        loggerArena = init_arena(4);
        init_error_vec();
    }
}


const char* log_level_to_string(LogLevel level) {
    switch(level) {
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_WARNING: return "WARNING";
        case LOG_LEVEL_INFO: return "INFO";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
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

void log_error_internal(ErrorPhase phase, ErrorCode code, const char* filepath, int line, size_t byte_offset, char* format, ...) {

    char* message = arena_alloc(loggerArena, 256 * sizeof (char));
    va_list args;
    va_start(args, format);
    vsnprintf(message, 256, format, args);
    va_end(args);


    PhasedError* error = (PhasedError*) arena_alloc(loggerArena, sizeof(PhasedError));
    if(phase == ERROR_PHASE_INTERNAL) {
        error->severity = ERROR_SEV_ERROR;
    } else {
        error->severity = ERROR_SEV_WARN;
    }

    error->code = code;
    error->phase = phase;
    error->file = filepath;
    error->line = line;
    error->name = get_filename_from_path(filepath);
    error->msg = message;

    if(error->severity == ERROR_SEV_WARN) {
        error->offending_code = get_line_from_file(filepath, byte_offset + 1, loggerArena);
        error->suggestion = error_code_to_suggestion(code);
    }

    log_message(LOG_LEVEL_ERROR, code, "%s:%d > %s\n",
                error->file, error->line, error->msg);

    push_error(error);

    if(error->severity == ERROR_SEV_ERROR) {
        print_all_errors();
        print_error_summary();
        exit(EXIT_FAILURE);
    }

}

char* get_line_from_file(const char* filepath, size_t byte_offset, Arena* arena) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        return NULL;
    }

    fseek(file, byte_offset, SEEK_SET);

    char buffer[256];
    int index = 0;
    char c;

    bool leading_spaces = true;

    while ((c = fgetc(file)) != '\n' && c != EOF && index < 255) {
        if (leading_spaces && (c == ' ' || c == '\t')) {
            continue;  // Skip leading spaces/tabs
        } else {
            leading_spaces = false;
            buffer[index++] = c;
        }
    }

    // Strip trailing spaces/tabs/carriage returns
    while (index > 0 && (buffer[index - 1] == ' ' || buffer[index - 1] == '\t' || buffer[index - 1] == '\r')) {
        index--;
    }
    buffer[index] = '\0';

    fclose(file);

    char* line = (char*) arena_alloc(arena, index + 1);
    strcpy(line, buffer);

    return line;
}


const char* get_filename_from_path(const char* filepath) {
    const char* filename = strrchr(filepath, '/');
    #ifdef _WIN32
        if (!filename) filename = strrchr(filepath, '\\');
    #endif
    return filename ? filename + 1 : filepath;
}


void close_log_file()
{
    if (log_file != NULL)
    {
        fclose(log_file);
        log_file = NULL;
    }
}

void destroy_logger_arena() {
    if (loggerArena) {
        destroy_arena(loggerArena);
        loggerArena = NULL;
    }
}
