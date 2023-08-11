#include "stack.h"
#include "logger.h"
#include <string.h>

Stack* init_stack(Arena* arena, size_t capacity) {
    Stack* stack = arena_alloc(arena, sizeof(Stack));
    if (!stack) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                  "Could not allocate memory for stack");
        return NULL;
    }

    stack->data = arena_alloc(arena, capacity * sizeof(void*));
    if (!stack->data) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                  "Could not allocate memory for stack data");
        return NULL;
    }

    stack->top = 0;
    stack->capacity = capacity;
    stack->arena = arena;
    return stack;
}

bool stack_push(Stack* s, void* value) {
    if (s->top == s->capacity) {
        log_error(ERROR_BUFFER_FULL, __FILE__, __LINE__,
                  "Stack is full, cannot push");
        return false;
    }

    s->data[s->top++] = value;
    return true;
}

bool stack_pop(Stack* s, void** value) {
    if (s->top == 0) {
        log_error(ERROR_BUFFER_EMPTY,
                  __FILE__, __LINE__, "Stack is empty, cannot pop");
        return false;
    }

    *value = s->data[--s->top];
    return true;
}

void* stack_peek(const Stack* s) {
    if (s->top == 0) {
        log_error(ERROR_BUFFER_EMPTY, __FILE__, __LINE__,
                  "Stack is empty, cannot peek");
        return NULL;
    }

    return s->data[s->top - 1];
}

bool stack_is_empty(const Stack* s) {
    return s->top == 0;
}

void stack_reset(Stack* s) {
    // ↓ Technically not necessary if you access the stack through only safe ways
    //memset(s->data, 0, sizeof(void*) * s->capacity);
    s->top = 0;
}