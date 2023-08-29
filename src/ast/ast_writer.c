#include "ast.h"
#include <string.h>

struct LabelCounter {
    char* labelPrefix;
    int counter;
};


Command symbol_to_command(char symbol) {
    switch (symbol) {
#define COMMAND(com, string, chr_repr) case char_repr : return com,
#include PATH_TO_WR_DEF_FILE
#undef COMMAND
    default:
       return COM_NONE;
    }
}

const char* command_to_string(Command command) {
    switch (command) {
#define COMMAND(com, string, char_repr) case com : return string;
#define COMMAND_ENUM
#include PATH_TO_WR_DEF_FILE
#undef COMMAND_ENUM
#undef COMMAND
        default:
           return "Unknown command";
    }
}

const char* segment_to_string(Segment segment) {
    switch (segment) {
#define SEGMENT(seg, string) case seg : return string;
#define SEGMENT_ENUM
#include PATH_TO_WR_DEF_FILE
#undef SEGMENT_ENUM
#undef SEGMENT
        default:
            return "Unknown segment";
    }
}

Segment kind_to_segment(Kind kind) {
    switch (kind) {
        case KIND_STATIC:
            return SEG_STATIC;
        case KIND_FIELD:
            return SEG_THIS;
        case KIND_ARG:
            return SEG_ARG;
        case KIND_VAR:
            return SEG_LOCAL;
        default:
            return SEG_NONE;
    }
}

void write_push(FILE* fptr, Segment segment, int index) {
    if(segment == SEG_NONE) {
        log_error_no_offset(ERROR_PHASE_CODEGEN, ERROR_INVALID_INPUT, __FILE__, __LINE__,
                            "['%s'] : SEG_NONE is invalid input", __func__);
    }
    fprintf(fptr, "push %s %d\n", segment_to_string(segment), index);
}

void write_pop(FILE* fptr, Segment segment, int index) {
    if(segment == SEG_NONE) {
          log_error_no_offset(ERROR_PHASE_CODEGEN, ERROR_INVALID_INPUT, __FILE__, __LINE__,
                            "['%s'] : SEG_NONE is invalid input", __func__);
    }
    fprintf(fptr, "pop %s %d\n", segment_to_string(segment), index);
}

void write_arithmetic(FILE* fptr, Command command) {
    if(command == COM_NONE) {
          log_error_no_offset(ERROR_PHASE_CODEGEN, ERROR_INVALID_INPUT, __FILE__, __LINE__,
                            "['%s'] : COM_NONE is invalid input", __func__);
    }
    fprintf(fptr, "%s\n", command_to_string(command));
}

void write_label(FILE* fptr, char* label) {
    fprintf(fptr, "label %s\n", label);
}

void write_goto(FILE* fptr, char* label) {
    fprintf(fptr, "goto %s\n", label);
}

void write_if(FILE* fptr, char* label) {
    fprintf(fptr, "if-goto %s\n", label);
}

void write_call(FILE* fptr, char* name, int n_args) {
    fprintf(fptr, "call %s %d\n", name, n_args);
}

void write_function(FILE* fptr, char* name, int n_locals) {
    fprintf(fptr, "function %s %d\n", name, n_locals);
}

void write_return(FILE* fptr) {
    fprintf(fptr, "return\n");
}

char* generate_unique_label(ASTVisitor* visitor, const char* labelPrefix) {
    if (!visitor || !labelPrefix) {
        log_error_no_offset(ERROR_PHASE_CODEGEN, ERROR_NULL_POINTER, __FILE__, __LINE__,
                            "['%s'] : visitor or labelPrefix are NULL", __func__);
        return NULL;
    }

    int counter = -1;

    for (int i = 0; i < vector_size(visitor->labelCounters); i++) {
        LabelCounter* lc = (LabelCounter*)vector_get(visitor->labelCounters, i);
        if (strcmp(lc->labelPrefix, labelPrefix) == 0) {
            counter = ++lc->counter;
            break;
        }
    }

    if (counter == -1) {
        LabelCounter* new = arena_alloc(visitor->arena, sizeof(LabelCounter));
        new->labelPrefix = arena_strdup(visitor->arena, labelPrefix);
        new->counter = 0;
        vector_push(visitor->labelCounters, new);
        counter = 0;
    }

    return arena_sprintf(visitor->arena, "%s_%d", labelPrefix, counter);
}

