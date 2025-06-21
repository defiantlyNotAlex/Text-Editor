#ifndef UNDO_H_
#define UNDO_H_

#include "short_types.h"
#include "stringbuilder.h"
#include "arena.h"
#include "arraylist.h"

typedef struct Transaction {
    isize line;
    isize col;

    String modified;
    bool removed;
} Transaction;

typedef struct Command {
    Transaction* data;
    isize count;
    isize capacity;
} Command;

typedef struct CommandList {
    Command* data;
    isize capacity;

    isize curr;
    isize end;
    
    Arena string_stack;

    bool unfinished_command;
} CommandList;

void reset_command(CommandList* commands);

void append_transaction(CommandList* commands, Transaction transaction);

void begin_command(CommandList* commands);
void end_command(CommandList* commands);

#endif //UNDO_H_