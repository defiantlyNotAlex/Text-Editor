#include "short_types.h"
#include "arena.h"
#include "stringbuilder.h"
#include "text.h"
#include "arraylist.h"

typedef struct Command {
    isize line;
    isize col;

    isize len;
    String data;
    bool insert;
} Command;

typedef struct CommandList {
    isize curr;
    Command* commands;
    
    Arena string_stack;
} CommandList;

void undo_command(Text* txt, CommandList* commands) {
    if (commands->curr < 0) {
        return;
    }
    commands->curr--;
    Command command = commands->commands[commands->curr];
    text_cursor_moveto(txt, command.col, command.line);
    if (command.insert) {
        text_cursor_remove_after(txt, command.len);
    } else {
        text_cursor_insert(txt, command.data.data, command.data.count);
    }
}
void redo_command(Text* txt, CommandList* commands) {
    if (commands->curr >= arrlist_count(commands->commands)) {
        return;
    }
    Command command = commands->commands[commands->curr];
    text_cursor_moveto(txt, command.col, command.line);
    if (command.insert) {
        text_cursor_insert(txt, command.data.data, command.data.count);
    } else {
        text_cursor_remove_after(txt, command.len);
    }
    commands->curr++;
}