#ifndef ERROR_H
#define ERROR_H

#include "stdbool.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define PATH_TO_ERR_DEF_FILE TOSTRING(DEF_FILES_DIR/error_defs.def)

#define ERROR_CODE(code, string, suggestion) code,
#define ERROR_CODE_ENUM
typedef enum {
    #include PATH_TO_ERR_DEF_FILE
} ErrorCode;
#undef ERROR_CODE_ENUM

#define ERROR_SEVERITY(code, string) code,
#define ERROR_SEVERITY_ENUM
typedef enum {
    #include PATH_TO_ERR_DEF_FILE
} ErrorSeverity;
#undef ERROR_SEVERITY_ENUM

#define ERROR_PHASE(code, string) code,
#define ERROR_PHASE_ENUM
typedef enum {
    #include PATH_TO_ERR_DEF_FILE
} ErrorPhase;
#undef ERROR_PHASE_ENUM

typedef struct {
    ErrorCode code;
    char *msg;
    const char *file;
    int line;
} Error;

typedef struct {
    ErrorCode code;
    ErrorPhase phase;
    ErrorSeverity severity;
    char* msg;
    const char* file;
    const char* name;
    int line;
    char* offending_code;
    char* suggestion;
} PhasedError;


Error get_error(void);

const char* error_code_to_string(ErrorCode code);
const char* error_code_to_suggestion(ErrorCode code);
const char *error_phase_to_string(ErrorPhase phase);
const char *error_severity_to_string(ErrorSeverity severity);
void init_error_vec();
void push_error(PhasedError *error);
int error_count();
int warning_count();
bool has_fatal_errors();
void clear_errors();
void print_error(PhasedError *error);
void print_all_errors();
void print_error_summary();
void destroy_error_vector();
void set_error(Error error);

#endif // ERROR_H