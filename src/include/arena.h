#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

typedef struct Arena Arena;

Arena* init_arena(size_t mutliplier);
void* arena_alloc(Arena* arena, size_t size);
void reset_arena(Arena* arena);
char* arena_sprintf(Arena* arena, const char* format, ...);
char* arena_strdup(Arena* arena, const char* src);
void destroy_arena(Arena* arena);

#endif // ARENA_ALLOCATOR_H