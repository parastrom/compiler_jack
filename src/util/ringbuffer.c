#include "ringbuffer.h"
#include "logger.h"

RingBuffer* init_ringbuffer() {
    RingBuffer* buffer = malloc(sizeof(RingBuffer));
    if (buffer == NULL) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] :  Could not allocate memory for the ringbuffer", __func__);
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
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_BUFFER_FULL, __FILE__, __LINE__,
                            "['%s'] : Ringbuffer full, cannot push", __func__);
        return false;
    }

    rb->data[rb->write_idx] = value;
    rb->write_idx = next_write_idx;
    return true;
}


bool ringbuffer_pop(RingBuffer* rb, Token** value) {
    if (rb->read_idx == rb->write_idx) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_BUFFER_EMPTY, __FILE__, __LINE__,
                            "['%s'] : Ringbuffer empty, cannot pop", __func__);
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
    if (rb_is_empty(rb)) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_BUFFER_EMPTY, __FILE__, __LINE__,
                            "['%s'] : Ringbuffer empty, cannot peek", __func__);
        return NULL;
    }
    return rb->data[rb->read_idx];
}

void ringbuffer_destroy(RingBuffer* rb) {
    if (rb == NULL) {
       log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_NULL_POINTER, __FILE__, __LINE__,
                            "['%s'] : rb is NULL", __func__);
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

bool rb_is_empty(const RingBuffer* rb) {
    return rb->read_idx == rb->write_idx;
}