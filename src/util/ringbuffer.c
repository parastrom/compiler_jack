#include "ringbuffer.h"
#include "logger.h"

RingBuffer* init_ringbuffer() {
    RingBuffer* buffer = malloc(sizeof(RingBuffer));
    if (buffer == NULL) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for ring buffer\n");
        return NULL;
    }
    buffer->write_idx = 0;
    buffer->read_idx = 0;
    return buffer;
}

bool ringbuffer_push(RingBuffer* rb, Token* value) {
    size_t next_write_idx = rb->write_idx + 1;
    if (next_write_idx >= RINGBUFFER_SIZE) {
        next_write_idx = 0;
    }
    if (next_write_idx == rb->read_idx) {
        log_error(ERROR_BUFFER_FULL, __FILE__, __LINE__, "Ring buffer is full, cannot push\n");
        return false;
    }

    rb->data[rb->write_idx] = value;
    rb->write_idx = next_write_idx;
    return true;
}


bool ringbuffer_pop(RingBuffer* rb, Token** value) {
    if (rb->read_idx == rb->write_idx) {
        log_error(ERROR_BUFFER_EMPTY, __FILE__, __LINE__, "Ring buffer is empty, cannot pop\n");
        return false;
    }

    *value = rb->data[rb->read_idx];
    size_t next_read_idx = rb->read_idx + 1;
    if (next_read_idx >= RINGBUFFER_SIZE) {
        next_read_idx = 0;
    }
    rb->read_idx = next_read_idx;
    return true;
}

Token* ringbuffer_peek(const RingBuffer* rb) {
    if (rb->read_idx == rb->write_idx) {
        log_error(ERROR_BUFFER_EMPTY, __FILE__, __LINE__, "Ring buffer is empty, cannot peek\n");
        return NULL;
    }
    return rb->data[rb->read_idx];
}

void ringbuffer_destroy(RingBuffer* rb) {
    if (rb == NULL) {
        log_error(ERROR_NULL_POINTER, __FILE__, __LINE__, "Ring buffer is NULL, cannot destroy\n");
        return;
    }

    // Free all tokens in the buffer
    while (rb->read_idx != rb->write_idx) {
        Token* token;
        ringbuffer_pop(rb, &token);
        destroy_token(token);
    }

    free(rb);
}