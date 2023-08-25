#ifndef TOKEN_QUEUE_H
#define TOKEN_QUEUE_H

#include "vector.h"
#include <stdio.h>
#include "token.h"
#include "arena.h"

typedef struct {
    int idx;
    vector list;
} TokenQueue;

TokenQueue * queue_init(Arena* arena);
bool queue_push(TokenQueue* queue, Token* ptr);
bool queue_pop(TokenQueue* queue, Token** val);
Token* queue_peek(const TokenQueue* queue);
Token* queue_peek_offset(TokenQueue *queue, int offset);

#endif //TOKEN_QUEUE_H