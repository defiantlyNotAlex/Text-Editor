#define ARENA_IMPLEMENTATION
#include "arena.h"

int main() {
    Arena a = {0};

    arena_alloc(&a, 10, 10, 1);
    arena_alloc(&a, 10, 10, 1);
    arena_alloc(&a, 10, 10, 1);

    arena_debug(&a);

    arena_pop(&a, 10, 10);

    arena_debug(&a);
    arena_pop(&a, 10, 10);

    arena_debug(&a);


    printf("Done!\n");
}