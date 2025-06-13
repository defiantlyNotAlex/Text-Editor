#ifndef TEXT_H_
#define TEXT_H_
#include "gapbuffer.h"
#include "undo.h"


typedef struct Text {
    StringBuilder filename;
    GapBuffer gapbuf;
    CommandList commands;

    isize* line_offsets;

    bool selected;
    isize selection_begin;
    isize selection_end;

    isize cursor_col;
    isize cursor_row;
} Text;

typedef struct CursorPosition {
    isize col;
    isize row;
} CursorPosition;

CursorPosition text_get_pos(Text* txt, isize index);
isize text_get_row(Text* txt, isize index);
isize text_index(Text* txt, isize col, isize row);

isize text_cursor_idx(Text* txt);

void text_cursor_update_position(Text* txt);
void text_cursor_move(Text* txt, isize n);
void text_cursor_move_codepoints(Text* txt, isize ncodepoints);
void text_cursor_moveto(Text* txt, isize col, isize row);
void text_cursor_move_until(Text* txt, bool forwards, bool (*predicate)(Codepoint c));

void text_cursor_move_to_selected(Text* txt, bool front);

void text_cursor_insert(Text* txt, String insert);
void text_cursor_remove_before(Text* txt, isize n);
void text_cursor_remove_after(Text* txt, isize n);

void text_add_transaction(Text* txt, String modified, bool removed);
void text_begin_command(Text* txt);
void text_end_command(Text* txt);

void text_undo(Text* txt);
void text_redo(Text* txt);

void text_update_line_offsets(Text* txt);
void text_cursor_update_position(Text* txt);

void text_delete_selection(Text* txt);
void text_copy_selection_to_clipboard(Text* txt);
void text_copy_and_delete_selection_to_clipboard(Text* txt);

void text_select_begin(Text* txt);
void text_select_end(Text* txt);
GapBufSlice text_selected_string(Text* txt);

void text_save_file(Text* txt);
void text_load_file(Text* txt, const char* filename);
void text_prompt_filename(StringBuilder* sb);

#endif //TEXT_H_