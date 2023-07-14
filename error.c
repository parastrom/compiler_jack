#ifndef ERROR_H
#define ERROR_H

#define MAX_ERRORS 100

#include "headers/error.h"
#include <stddef.h>

static Error errors[MAX_ERRORS];
static int num_errors = 0;

void set_error(Error error) {
    if(num_errors < MAX_ERRORS) {
        errors[num_errors] = error;
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