#ifndef UNDO_H_
#define UNDO_H_

#include "short_types.h"
#include "stringbuilder.h"
#include "arena.h"
#include "arraylist.h"

typedef struct SubCommand {
    isize line;
    isize col;

    String inserted;
    String removed;
} SubCommand;

typedef struct Command {
    SubCommand* sub_commands;
} Command;

typedef struct CommandList {
    isize curr;
    Command* commands;
    
    Arena inserted_stack;
    Arena removed_stack;
    Arena command_stack;
    bool unfinished_command;
} CommandList;

void reset_command(CommandList* commands);
void insert_command(CommandList* commands, String inserted, String removed, isize col, isize row);

void begin_command(CommandList* commands);
void end_command(CommandList* commands);


#endif //UNDO_H_