#ifndef UNDO_H_
#define UNDO_H_

#include "short_types.h"
#include "stringbuilder.h"
#include "arena.h"
#include "arraylist.h"

typedef struct Command {
    isize line;
    isize col;

    String inserted;
    String removed;
} Command;

typedef struct CommandList {
    isize curr;
    Command* commands;
    
    Arena inserted_stack;
    Arena removed_stack;
} CommandList;

void reset_command(CommandList* commands);
void insert_command(CommandList* commands, String inserted, String removed, isize col, isize row);
Command pop_command(CommandList* commands);
void update_command(CommandList* commands, String inserted_new, String removed_new);

#endif //UNDO_H_