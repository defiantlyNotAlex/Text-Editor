#ifndef ARENA_H_
#define ARENA_H_

#include <stdint.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdarg.h>

#define ARENA_REGION_SIZE 200 // 4 KiB //1 MiB
typedef struct ArenaRegion ArenaRegion;

// dynamically expanding arena implementation, zeroes memory by default
// keeps track of how much memory has been allocated
// absolutely not thread safe
// implemented as a linked list of ArenaRegions
// can clear while preserving allocations with arena_clear
// can pass by value to get a scratch Arena where it will forget everything the scratch allocated
// but preserve any new allocations made by the scratch
typedef struct Arena {
    ArenaRegion* begin;
    ArenaRegion* head;
} Arena;

// doubly linked list of ArenaRegions which each have ARENA_REGION_SIZE bytes of memory
struct ArenaRegion {
    size_t capacity;
    size_t used;
    ArenaRegion* next; // next region (NULL if it is the end)
    ArenaRegion* prev;

    uint8_t begin[];
};

ArenaRegion* arena_create_region(size_t capacity);

void arena_init(Arena* arena);
void arena_free(Arena* arena);
void arena_clear(Arena* arena);

void* arena_alloc(Arena* arena, size_t count, size_t size, size_t align);
void* arena_realloc(Arena* arena, void* oldPtr, size_t oldCount, size_t newCount, size_t size, size_t align);
void arena_pop(Arena* arena, size_t count, size_t size);

void* arena_memdup(Arena* arena, const void* src, size_t size, size_t align);
char* arena_strdup(Arena* arena, const char* str);
char* arena_sprintf(Arena* arena, const char* fmt, ...);

#include "stringbuilder.h"
String arena_string_dup(Arena* arena, String string);

#define arena_new(arena, count, type) (type*)arena_alloc(arena, count, sizeof(type), alignof(type))
#define arena_expand(arena, ptr, oldCount, newCount, type) (type*)arena_realloc(arena, ptr, oldCount, newCount, sizeof(type), alignof(type))

#define arena_clone(arena, reference) arena_memdup(arena, reference, sizeof(*reference), alignof(*reference))

#ifdef ARENA_IMPLEMENTATION
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

void arena_debug(Arena* arena) {
    ArenaRegion* curr = arena->begin;
    while (curr != NULL) {
        if (curr == arena->head) printf("[HEAD]");
        else printf("      ");
        printf("[%p], next: %p, prev: %p, (%llu / %llu)\n", curr, curr->next, curr->prev, curr->used, curr->capacity);
        curr = curr->next;
    }
}

// creates a region of memory for the arena
ArenaRegion* arena_create_region(size_t capacity) {
    ArenaRegion* region = malloc(capacity + sizeof(ArenaRegion));
    assert(region && "malloc failed");

    region->next = NULL;
    region->prev = NULL;
    region->capacity = capacity;
    region->used = 0;
    return region;
}
// initialises the arena with a starting region
void arena_init(Arena* arena) {
    arena->begin = arena_create_region(ARENA_REGION_SIZE);
    arena->head = arena->begin;
}
// frees all heap allocations associated with the arena
void arena_free(Arena* arena) {
    ArenaRegion* curr = arena->begin;
    while (curr) {
        ArenaRegion* temp = curr;
        curr = curr->next;
        free(temp);
    }
    arena->begin = NULL;
    arena->head = NULL;
}
// clears the arena while maintaining all allocations
// it does this by setting head to begin and used to 0 which makes the arena
// effectively forget what it has allocated
void arena_clear(Arena* arena) {
    arena->head = arena->begin;
    ArenaRegion* curr = arena->begin;
    while (curr) {
        curr->used = 0;
        curr = curr->next;
    }
}

// allocates zeroed memory, aligns the head
// expands the arena if it runs out of memory
void* arena_alloc(Arena* arena, size_t count, size_t size, size_t align) {
    if (!arena->head) arena_init(arena);
    
    ArenaRegion* region = arena->head;
    
    ptrdiff_t padding = -(uintptr_t)((region->begin + region->used)) & (align - 1);
    ptrdiff_t free_space = region->capacity - region->used;
    ptrdiff_t space_needed = count * size + padding;
    if (free_space < space_needed) {
        // attempting to allocate more than ARENA_REGION_SIZE
        if (count * size >= ARENA_REGION_SIZE) {
            ArenaRegion* new = arena_create_region(count * size + ARENA_REGION_SIZE);

            arena->head = new;
            new->next = NULL;
            new->prev = region;
            region->next = new;
        }

        arena->head = arena->head->next;
        if (arena->head == NULL) {
            arena->head = arena_create_region(ARENA_REGION_SIZE);
            region->next = arena->head;
            arena->head->prev = region;
        }
        padding = -(uintptr_t)((region->begin + region->used)) & (align - 1);
        region = arena->head;
    }
    
    void* ptr = (region->begin + region->used + padding);
    region->used += space_needed;
    memset(ptr, 0, count * size);
    
    return ptr; 
}

// reallocates the memory stored at oldPtr
// if the allocation is at the head of the arena it will attempt to modify inplace
// if it doesnt have enough room to modify in place or it is not at the head
// it will allocate a new block and copy the memory over (zero initialised any new memory)
void* arena_realloc(Arena* arena, void* oldPtr, size_t oldCount, size_t newCount, size_t size, size_t align) {
    if (!arena->head) arena_init(arena);
    
    if (!oldPtr) return arena_alloc(arena, newCount, size, align);
    
    assert(newCount * size <= ARENA_REGION_SIZE && "allocations must be smaller than a whole block");
    ArenaRegion* region = arena->head;
    if (region->begin + region->used == (uint8_t*)oldPtr + oldCount * size) {
        ptrdiff_t spaceRequired = (newCount - oldCount) * size;
        if (spaceRequired <= (ptrdiff_t)region->capacity - (ptrdiff_t)region->used) {
            region->used += (newCount - oldCount) * size;

            return oldPtr;
        }
    }
    void* newPtr = arena_alloc(arena, newCount, size, align);
    memcpy(newPtr, oldPtr, oldCount * size);
    
    return newPtr;
}
// frees count * size bytes from the end of the arena
void arena_pop(Arena* arena, size_t count, size_t size) {
    ArenaRegion* region = arena->head;
    if (region == NULL) return;

    size_t space_to_free = count * size;
    while (space_to_free > 0) {
        if (region->used > space_to_free) {
            region->used -= count * size;
            return;
        } else {
            space_to_free -= region->used;
            region->used = 0;
            region = region->prev;
            if (region == NULL) return;
            arena->head = region;
        }        
    }
}

// duplicates what is stored in src
void* arena_memdup(Arena* arena, const void* src, size_t size, size_t align) {
    void* ptr = arena_alloc(arena, 1, size, align);
    memcpy(ptr, src, size);
    return ptr;
}

// equivelent to stdlib strdup except it is allocated in an arena
char* arena_strdup(Arena* arena, const char* str) {
    size_t len = strlen(str);
    char* new_string = arena_alloc(arena, len + 1, 1, 1);
    memcpy(new_string, str, len + 1);
    return new_string;
}

char* arena_sprintf(Arena* arena, const char* fmt, ...) {
    va_list args;
    va_start (args, fmt);
    ptrdiff_t len = vsnprintf("", 0, fmt, args);
    if (len < 0) {
        // error
        return NULL;
    }
    // len + 1 bc vsnprintf doesn't include the null terminator in the length
    char* str = arena_alloc(arena, len + 1, 1, 1); 
    vsnprintf(str, len + 1, fmt, args);
    va_end (args);
    return str;
}

String arena_string_dup(Arena* arena, String string) {
    String result = {.data = arena_memdup(arena, string.data, string.count, 1), .count = string.count};
    return result;
}

#endif // ARENA_IMPLEMENTATION
#endif // ARENA_H_