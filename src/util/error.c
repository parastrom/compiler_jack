#define MAX_ERRORS 100
#define STACK_CAPACITY 128

#include "error.h"
#include <stddef.h>
#include "vector.h"
#include <stdio.h>

static Error errors[MAX_ERRORS];
static int num_errors = 0;
static vector errorVector = NULL;


void init_error_vec() {
   errorVector = vector_create();
}

void push_error(PhasedError *error) {
    vector_push(errorVector, error);
}

int error_count() {
    return vector_size(errorVector);
}

int warning_count() {
    int count = 0;
    for (int i = 0; i < vector_size(errorVector); i++) {
        PhasedError *error = (PhasedError *)vector_get(errorVector, i);
        if (error->severity == ERROR_SEV_WARN) {
            count++;
        }
    }
    return count;
}

bool has_fatal_errors() {
    return error_count() > 0;
}

void clear_errors() {
    while (vector_size(errorVector) > 0) {
        vector_pop(errorVector);
    }
}

void print_error(PhasedError *error) {
    if (!error) {
        fprintf(stderr, "Received a NULL error.\n");
        return;
    }

    fprintf(stderr, "[%s][%s:%s]: ", error_severity_to_string(error->severity),
            error_phase_to_string(error->phase), error_code_to_string(error->code));
    fprintf(stderr, "In file '%s' , line %d:\n", error->name, error->line + 1);
    fprintf(stderr, "\t%s\n", error->msg);

    if (error->offending_code) {
        fprintf(stderr, "source : %s\n", error->offending_code);
    }

    if(error->suggestion) {
        fprintf(stderr, "hint : %s\n", error->suggestion);
    }
    fprintf(stderr, "path: %s\n", error->file);


}

void print_all_errors() {
      for (int i = 0; i < vector_size(errorVector); i++) {
          PhasedError* error = (PhasedError*) vector_get(errorVector, i);
          print_error(error);
      }
}

void print_error_summary() {
    int totalErrors = error_count();
    int totalWarnings = warning_count();
    bool fatal = has_fatal_errors();

    fprintf(stderr, "========== Error Summary ==========\n");
    fprintf(stderr, "Total Errors: %d\n", totalErrors);
    fprintf(stderr, "Total Warnings: %d\n", totalWarnings);
    if (fatal) {
        fprintf(stderr, "Compilation terminated due to fatal errors and/or warnings.\n");
    } else {
        fprintf(stderr, "Compilation was successful.\n");
    }
    fprintf(stderr, "===================================\n");
}


const char *error_severity_to_string(ErrorSeverity severity) {
    switch (severity) {
#define ERROR_SEVERITY(code, string) case code: return string;
#define ERROR_SEVERITY_ENUM

#include PATH_TO_ERR_DEF_FILE
#undef ERROR_SEVERITY_ENUM
#undef ERROR_SEVERITY
        default:
            return "Unknown severity";
    }
}

const char *error_phase_to_string(ErrorPhase phase) {
    switch (phase) {
#define ERROR_PHASE(code, string) case code: return string;
#define ERROR_PHASE_ENUM

#include PATH_TO_ERR_DEF_FILE
#undef ERROR_PHASE_ENUM
#undef ERROR_PHASE
        default:
            return "Unknown phase";
    }
}

const char* error_code_to_string(ErrorCode code) {
    switch (code) {
#define ERROR_CODE(code, string, suggestion) case code: return string;
#define ERROR_CODE_ENUM

#include PATH_TO_ERR_DEF_FILE
#undef ERROR_CODE_ENUM
#undef ERROR_CODE
        default:
            return "Unknown code";
    }
}


const char* error_code_to_suggestion(ErrorCode code) {
    switch (code) {
#define ERROR_CODE(code, string, suggestion) case code: return suggestion;
#define ERROR_CODE_ENUM

#include PATH_TO_ERR_DEF_FILE
#undef ERROR_CODE_ENUM
#undef ERROR_CODE
        default:
            return "No suggestion available.";
    }
}

void destroy_error_vector() {
    vector_destroy(errorVector);
}


