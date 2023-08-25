#include "token_queue.h"
#include "logger.h"

TokenQueue *queue_init(Arena *arena) {
    TokenQueue *queue = arena_alloc(arena, sizeof(TokenQueue));

    if (!queue) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                  "Failed to allocate memory for TokenQueue");
        return NULL;
    }

    queue->idx = 0;
    queue->list = vector_create();

    if (!queue->list) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                  "Failed to create vector for TokenQueue");
        return NULL;
    }

    return queue;
}

bool queue_push(TokenQueue *queue, Token *ptr) {
    if (!queue || !ptr) {
        log_error(ERROR_NULL_POINTER, __FILE__, __LINE__,
                  "Null pointer provided to queue_push");
        return false;
    }

    vector_push(queue->list, ptr);
    return true;
}

bool queue_pop(TokenQueue *queue, Token **val) {
    if (!queue || !val) {
        log_error(ERROR_NULL_POINTER, __FILE__, __LINE__,
                  "Null pointer provided to queue_pop");
        return false;
    }

    if (queue->idx >= vector_size(queue->list)) {
        log_error(ERROR_OUT_OF_BOUNDS, __FILE__, __LINE__,
                  "Attempted to pop beyond the end of the queue");
        return false;
    }

    *val = vector_get(queue->list, queue->idx);
    queue->idx++;
    return true;
}

Token *queue_peek(const TokenQueue *queue) {
    if (!queue) {
        log_error(ERROR_NULL_POINTER, __FILE__, __LINE__,
                  "Null pointer provided to queue_peek");
        return NULL;
    }

    if (queue->idx < vector_size(queue->list)) {
        return vector_get(queue->list, queue->idx);
    }

    return NULL;
}


Token* queue_peek_offset(TokenQueue *queue, int offset) {
    if (!queue) {
        log_error(ERROR_NULL_POINTER, __FILE__, __LINE__,
                  "Null pointer provided to queue_peek");
        return NULL;
    }

    if (queue->idx + offset < vector_size(queue->list)) {
        return vector_get(queue->list, queue->idx + offset);
    }
    return NULL;
}

