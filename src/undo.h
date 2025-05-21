#ifndef UNDO_H_
#define UNDO_H_

#include "short_types.h"
#include "stringbuilder.h"
#include "arena.h"
#include "text.h"
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
    
    Arena string_stack;
} CommandList;

void reset_command(CommandList* commands);
void undo_command(Text* txt, CommandList* commands);
void redo_command(Text* txt, CommandList* commands);
void insert_command(Text* txt, CommandList* commands, String inserted, String removed, isize col, isize row);

#endif //UNDO_H_