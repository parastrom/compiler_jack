#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

typedef struct Arena Arena;

Arena* init_arena(size_t size);
void* arena_alloc(Arena* arena, size_t size);
static size_t get_page_size();
void reset_arena(Arena* arena);
void destroy_arena(Arena* arena);

#endif // ARENA_ALLOCATOR_H