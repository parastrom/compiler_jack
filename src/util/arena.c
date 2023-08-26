#include "arena.h"
#include <stdlib.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include "logger.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <unistd.h>
#endif

 struct Arena {
    void* start;
    void* current;
    void* committed_end;
    void* reserved_end;
};

#define PAGE_SIZE (get_page_size())
#define ALIGN_UP(pointer, alignment) \
    ((void*)(((uintptr_t)(pointer) + (alignment)-1) & ~((alignment)-1)))


static size_t get_page_size() {
    #ifdef _WIN32
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return si.dwPageSize;
    #else
        return sysconf(_SC_PAGESIZE);
    #endif
}

Arena* init_arena(size_t multiplier) {
    size_t size = multiplier * PAGE_SIZE;

    Arena* arena = malloc(sizeof(Arena));
    if (!arena) {
        return NULL;
    }

    #ifdef _WIN32
        arena->start = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    #else
        arena->start = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (arena->start == MAP_FAILED) {
            free(arena);
            return NULL;
        }
    #endif

    arena->current = arena->start;
    arena->committed_end = arena->start;
    arena->reserved_end = arena->start + size;
    return arena;
}



static bool commit_memory(void* addr, size_t size) {
    // Size, rounded to page boundaries.
    size_t commit_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    #ifdef _WIN32
        return VirtualAlloc(addr, commit_size, MEM_COMMIT, PAGE_READWRITE) != NULL;
    #else
        return mprotect(addr, commit_size, PROT_READ | PROT_WRITE) == 0;
    #endif
}

void* arena_alloc(Arena* arena, size_t size) {
    // Check if there's enough space
    if (!arena || arena->current + size > arena->reserved_end) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Arena out of space", __func__);
        return NULL;
    }

    // Align the current pointer to the system's word size
    arena->current = ALIGN_UP(arena->current, alignof(max_align_t));

    // Check if we need to commit more memory
    if (arena->current + size > arena->committed_end) {
        size_t needed_size = ((char*)arena->current + size) - (char*)arena->committed_end;
        size_t commit_size = (needed_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        if (!commit_memory(arena->committed_end, needed_size)) {
            log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to commit memory from the arena", __func__);
            return NULL;
        }
        arena->committed_end = (char*)arena->committed_end + commit_size;
    }


    void* result = arena->current;
    arena->current = (char*)arena->current + size;
    return result;
}


void reset_arena(Arena* arena) {
    if (!arena) {
         log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_NULL_POINTER, __FILE__, __LINE__,
                            "['%s'] : Null pointer provided", __func__);
        return;
    }
    arena->current = arena->start;
}

void destroy_arena(Arena* arena) {
    if (!arena) {
         log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_NULL_POINTER, __FILE__, __LINE__,
                            "['%s'] : Null pointer provided", __func__);
        return;
    }

    #ifdef _WIN32
        if (!VirtualFree(arena->start, 0, MEM_RELEASE)) {
            log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to free reserved memory from the arena", __func__);
        }
    #else
        if (munmap(arena->start, (char*)arena->reserved_end - (char*)arena->start) == -1) {
            log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to unmap memory arena", __func__);
        }
    #endif
    free(arena);
}
