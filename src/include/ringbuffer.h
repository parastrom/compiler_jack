#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdbool.h>
#include <stdio.h>
#include "token.h"

#define RINGBUFFER_SIZE 2048

typedef struct {
    Token* data[RINGBUFFER_SIZE];
    size_t read_idx;
    size_t write_idx;
} RingBuffer;

RingBuffer* init_ringbuffer();
bool ringbuffer_push(RingBuffer* buffer, Token* token);
bool ringbuffer_pop(RingBuffer* buffer, Token** value);
void ringbuffer_destroy(RingBuffer* rb);
bool rb_is_empty(const RingBuffer* rb);
Token* ringbuffer_peek(const RingBuffer* rb);

#endif // RINGBUFFER_H