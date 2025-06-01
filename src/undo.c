#include "undo.h"

String string_arena_dup(Arena* arena, String string) {
    String result = {.data = arena_memdup(arena, string.data, string.count, 1), .count = string.count};
    return result;
}
void reset_command(CommandList* commands) {
    arrlist_setcount(commands->commands, 0);
    commands->curr = 0;
    arena_clear(&commands->inserted_stack);
}
void insert_command(CommandList* commands, String inserted, String removed, isize col, isize row) {
    Command command = {
        .line = row,
        .col = col,

        .inserted = string_arena_dup(&commands->inserted_stack, inserted),
        .removed = string_arena_dup(&commands->removed_stack, removed),
    };
    if (arrlist_count(commands->commands) <= commands->curr) {
        arrlist_expand(commands->commands, 1);
    }
    commands->commands[commands->curr++] = command;     
    arrlist_setcount(commands->commands, commands->curr);
}
// add character
// remove charachter
