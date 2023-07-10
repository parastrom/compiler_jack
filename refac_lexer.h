#ifndef REFAC_LEXER_H
#define REFAC_LEXER_H

#include <stdio.h>
#include "token.h"

#define RINGBUFFER_SIZE 2048


typedef struct {
    Token* data[RINGBUFFER_SIZE];
    size_t read_idx;
    size_t write_idx;
} RingBuffer;

typedef struct {
    const char* input;
    size_t position;
    RingBuffer* queue;
};

RingBuffer* init_ringbuffer();
bool ringbuffer_push(RingBuffer* buffer, Token* token);
bool ringbuffer_pop(RingBuffer* buffer, Token** value);
void ringbuffer_destroy(RingBuffer* rb);
Token* ringbuffer_peek(RingBuffer* rb);




#endif  // REFAC_LEXER_H