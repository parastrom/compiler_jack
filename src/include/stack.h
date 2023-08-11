#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include "arena.h"

typedef struct {
    void** data;
    size_t top;
    size_t capacity;
    Arena* arena;
} Stack;

Stack* init_stack(Arena* arena, size_t capacity);
bool stack_push(Stack* s, void* value);
bool stack_pop(Stack* s, void** value);
void* stack_peek(const Stack* s);
bool stack_is_empty(const Stack* s);
void stack_reset(Stack* s);
#endif //STACK_H
