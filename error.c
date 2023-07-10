#ifndef ERROR_H
#define ERROR_H

#define MAX_ERRORS 100

#include "error.h";
#include <stddef.h>

static Error errors[MAX_ERRORS];
static int num_errors = 0;

void set_error(ErrorCode code, int line, const char* file, char* msg) {
    if(num_errors < MAX_ERRORS) {
        errors[num_errors].code = code;
        errors[num_errors].line = line;
        errors[num_errors].file = file;
        errors[num_errors].msg = msg;
        num_errors++;
    }
}


Error get_error(void) {
    if (num_errors > 0) {
        return errors[--num_errors];
    } else {
        Error e;
        e.code = ERROR_NONE;
        e.line = 0;
        e.file = NULL;
        e.msg = NULL;
        return e;
    }
}

#endif