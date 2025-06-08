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
    SubCommand command = {
        .line = row,
        .col = col,

        .inserted = string_arena_dup(&commands->inserted_stack, inserted),
        .removed = string_arena_dup(&commands->removed_stack, removed),
    };
    arrlist_append(commands->commands[commands->curr].sub_commands, command);
}
// finish unfinished commands
// create subcommand list
// pop all commands after itself
void begin_command(CommandList* command_list) {
    if (command_list->unfinished_command) end_command(command_list);
    command_list->unfinished_command = true;

    if (arrlist_count(command_list->commands) != command_list->curr) {
        // pops all commands after this one
        for (isize i = arrlist_count(command_list->commands) - 1; i > command_list->curr; i--) {
            for (isize j = 0; j < arrlist_count(command_list->commands[i].sub_commands); j++) {
                SubCommand* sub_comm = command_list->commands[i].sub_commands;
                arena_pop(&command_list->inserted_stack, sub_comm[i].inserted.count, 1);
                arena_pop(&command_list->removed_stack , sub_comm[i].removed.count , 1);
                arrlist_setcount(sub_comm, 0);
            }
        }
    }
    arrlist_setcount(command_list->commands, command_list->curr);
}
// increment curr
void end_command(CommandList* commands) {
    commands->curr++;
    arrlist_setcount(commands->commands, commands->curr);
    commands->unfinished_command = false;
}
// add character
// remove charachter
