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
void undo_command(Text* txt, CommandList* commands) {
    if (commands->curr <= 0) {
        return;
    }
    commands->curr--;
    Command command = commands->commands[commands->curr];
    text_cursor_moveto(txt, command.col, command.line);
    text_cursor_remove_after(txt, command.inserted.count);
    text_cursor_insert(txt, command.removed.data, command.removed.count);

    text_update_line_offsets(txt);
    text_cursor_update_position(txt);
}
void redo_command(Text* txt, CommandList* commands) {
    if (commands->curr >= arrlist_count(commands->commands)) {
        return;
    }
    Command command = commands->commands[commands->curr];
    text_cursor_moveto(txt, command.col, command.line);
    text_cursor_remove_after(txt, command.removed.count);
    text_cursor_insert(txt, command.inserted.data, command.inserted.count);
    commands->curr++;

    text_update_line_offsets(txt);
    text_cursor_update_position(txt);
}

void insert_command(Text* txt, CommandList* commands, String inserted, String removed, isize col, isize row) {
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
