#include "undo.h"
#include <stdlib.h>

void reset_command(CommandList* commands) {
    arena_clear(&commands->string_stack);
    for (isize i = 0; i < commands->end; i++) {
        free(commands->data[i].data);
    }
    memset(commands->data, 0, commands->end * sizeof(Command));
    commands->curr = 0;
    commands->end = 0;
}
void append_transaction(CommandList* commands, Transaction transaction) {
    Command* command = &commands->data[commands->curr];

    // out of capacity
    if (command->count == command->capacity) {
        isize new_cap = 8;
        if (new_cap < command->capacity * 2) new_cap = command->capacity * 2;
        command->data = realloc(command->data, new_cap * sizeof(Transaction));
        command->capacity = new_cap;
    }

    command->data[command->count++] = transaction;
}
void begin_command(CommandList* commands) {
    if (commands->unfinished_command) end_command(commands);
    commands->unfinished_command = true;

    // out of capacity
    if (commands->end == commands->capacity) {
        isize new_cap = 8;
        if (new_cap < commands->capacity * 2) new_cap = commands->capacity * 2;
        commands->data = realloc(commands->data, new_cap * sizeof(Command));
        commands->capacity = new_cap;
    }

    if (commands->end != commands->curr) {
        // pops all commands after this one
        isize size = 0;
        for (isize i = commands->curr; i < commands->end; i++) {
            Command* command = &commands->data[i];
            for (isize j = 0; j < command->count; j++) {
                Transaction* transaction = &commands->data[i].data[j];
                size += transaction->modified.count;
            }
            free(command->data);
            command->data = NULL;
            command->count = 0;
            command->capacity = 0;
        }
        arena_pop(&commands->string_stack, size, 1);
        commands->end = commands->curr;
    }
    commands->data[commands->curr].data = NULL;
    commands->data[commands->curr].count = 0;
    commands->data[commands->curr].capacity = 0;
}
void end_command(CommandList* commands) {
    commands->curr++;
    commands->end++;
    commands->unfinished_command = false;
}
