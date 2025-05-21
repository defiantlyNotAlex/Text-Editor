#include "text.h"
#include "arraylist.h"
#include <raylib.h>
#include <stdlib.h>

CursorPosition text_get_pos(Text* txt, isize index) {
    isize row = 0;
    isize curr = 0;
    for (row = 0; row < arrlist_count(txt->line_offsets); row++) {
        if (txt->line_offsets[row] > index) break;
        curr = txt->line_offsets[row];
    }
    GapBufSlice strings = gapbuf_getstrings(&txt->gapbuf);
    isize col = 0;
    while (curr < index) {
        if (curr < strings.l.count) {
            curr = string_iterate(strings.l, curr);
        } else {
            curr = string_iterate(strings.r, curr - strings.l.count) + strings.l.count;
        }
        col++;
    }
    CursorPosition pos = {
        .col = col,
        .row = row,
    };
    return pos;
}

isize text_get_row(Text* txt, isize index) {
    isize count;
    for (count = 0; count < arrlist_count(txt->line_offsets); count++) {
        if (txt->line_offsets[count] >= index) break;
    }
    return count;
}
void text_cursor_move(Text* txt, isize n) {
    if (txt->gapbuf.gap_end + n > txt->gapbuf.capacity) {\
        n = txt->gapbuf.capacity - txt->gapbuf.gap_end;
    } else if (txt->gapbuf.gap_begin + n < 0) {
        n = -txt->gapbuf.gap_begin;
    }
    gapbuf_movegap_rel(&txt->gapbuf, n);
}
void text_cursor_move_codepoints(Text* txt, isize ncodepoints) {
    GapBufSlice strings = gapbuf_getstrings(&txt->gapbuf);
    isize index = text_cursor_idx(txt);
    if (ncodepoints < 0) {
        ncodepoints = -ncodepoints;
        for (isize i = 0; i < ncodepoints && index > 0; i++) {
            index = string_iterate_back(strings.l, index);
        }
        
    } else {
        for (isize i = 0; i < ncodepoints && index < strings.l.count + strings.r.count; i++) {
            index = string_iterate(strings.r, index - strings.l.count) + strings.l.count;
        }
    }
    text_cursor_move(txt, index - text_cursor_idx(txt));
}

isize text_index(Text* txt, isize col, isize row) {
    if (row < 0) row = 0;
    if (row > arrlist_count(txt->line_offsets)) row = arrlist_count(txt->line_offsets);
    isize index = row == 0 ? 0 : txt->line_offsets[row - 1];
    GapBufSlice strings = gapbuf_getstrings(&txt->gapbuf);

    isize curr_col = 0;
    while (curr_col < col) {
        if (index < strings.l.count) {
            if (strings.l.data[index] == '\n') break;
            index = string_iterate(strings.l, index);
        } else if (index < strings.l.count + strings.r.count) {
            if (strings.r.data[index - strings.l.count] == '\n') break;
            index = string_iterate(strings.r, index - strings.l.count) + strings.l.count;
        } else {
            break;
        }
        curr_col++;
    }
    return index;
}
void text_cursor_update_position(Text* txt);
void text_cursor_moveto(Text* txt, isize col, isize row) {
    isize index = text_index(txt, col, row);
    text_cursor_move(txt, index - text_cursor_idx(txt));
    text_cursor_update_position(txt);
}

void text_cursor_move_until(Text* txt, bool forwards, bool (*predicate)(Codepoint c)) {
    GapBufSlice strings = gapbuf_getstrings(&txt->gapbuf);
    isize n = 0;
    bool hit = false;
    if (forwards) {
        for (isize i = 0; i < strings.r.count;) {
            Codepoint c = string_next_codepoint(strings.r, &i);
            if (predicate(c)) {
                n = i;
                hit = true;
                break;
            }
        }
        if (!hit) n = strings.r.count;
    } else {
        for (isize i = strings.l.count; i > 0;) {
            Codepoint c = string_prev_codepoint(strings.l, &i);
            if (predicate(c)) {
                n = -strings.l.count + i;
                hit = true;
                break;
            }
        }
        if (!hit) n = -strings.l.count;
    }
    text_cursor_move(txt, n);
    text_cursor_update_position(txt);
}

isize text_cursor_idx(Text* txt) {
    return txt->gapbuf.gap_begin;
}

void text_cursor_insert(Text* txt, const char* buf, isize n) {
    gapbuf_insertn(&txt->gapbuf, buf, n);
}
String text_cursor_remove_before(Text* txt, isize n) {
    gapbuf_removen(&txt->gapbuf, n);
    return (String) {.data = txt->gapbuf.data + txt->gapbuf.gap_begin, .count = n};
}
String text_cursor_remove_after(Text* txt, isize n) {
    gapbuf_removen_after(&txt->gapbuf, n);
    return (String) {.data = txt->gapbuf.data + txt->gapbuf.gap_end - n, .count = n};
}
void text_update_line_offsets(Text* txt) {
    arrlist_setcount(txt->line_offsets, 0);

    GapBufSlice strings = gapbuf_getstrings(&txt->gapbuf);
    isize index = 0;
    for (isize i = 0; i < strings.l.count; i++) {
        index++;
        if (strings.l.data[i] == '\n') arrlist_append(txt->line_offsets, index);
    }
    for (isize i = 0; i < strings.r.count; i++) {
        index++;
        if (strings.r.data[i] == '\n') arrlist_append(txt->line_offsets, index);
    }
}
void text_cursor_update_position(Text* txt) {
    CursorPosition pos = text_get_pos(txt, text_cursor_idx(txt));
    txt->cursor_col = pos.col;
    txt->cursor_row = pos.row;
}

String text_delete_selection(Text* txt) {
    txt->selected = false;
    isize l, r;
    if (txt->selection_begin < txt->selection_end) {
        l = txt->selection_begin;
        r = txt->selection_end;
    } else {
        l = txt->selection_end;
        r = txt->selection_begin;
    }
    text_cursor_move(txt, l - text_cursor_idx(txt));
    text_cursor_remove_after(txt, r - l);
    txt->selection_begin = l;

    return (String){.data = txt->gapbuf.data + txt->gapbuf.gap_begin, r - l};
}
void text_copy_selection_to_clipboard(Text* txt) {
    isize l, r;
    if (txt->selection_begin < txt->selection_end) {
        l = txt->selection_begin;
        r = txt->selection_end;
    } else {
        l = txt->selection_end;
        r = txt->selection_begin;
    }
    isize count = r - l;

    GapBufSlice slice = gapbuf_slice(&txt->gapbuf, l, r);
    char* buffer = malloc(count + 1);
    memcpy(buffer, slice.l.data, slice.l.count);
    memcpy(buffer + slice.l.count, slice.r.data, slice.r.count);
    buffer[count] = '\0';
    SetClipboardText(buffer);
    free(buffer);
}

void text_select_begin(Text* txt) {
    txt->selected = true;
    txt->selection_begin = text_index(txt, txt->cursor_col, txt->cursor_row);
}
void text_select_end(Text* txt) {
    txt->selection_end = text_index(txt, txt->cursor_col, txt->cursor_row);
}

GapBufSlice text_selected_string(Text* txt) {
    isize l, r;
    if (txt->selection_begin < txt->selection_end) {
        l = txt->selection_begin;
        r = txt->selection_end;
    } else {
        l = txt->selection_end;
        r = txt->selection_begin;
    }
    return gapbuf_slice(&txt->gapbuf, l, r);
}

void text_save_file(Text* txt) {
    if (txt->filename.count == 0) {
        text_prompt_filename(&txt->filename);
    }
    gapbuf_write_entire_file(&txt->gapbuf, txt->filename.data);
    text_update_line_offsets(txt);
    text_cursor_update_position(txt);
}
void text_load_file(Text* txt, const char* filename) {
    gapbuf_read_entire_file(&txt->gapbuf, filename);
    string_clear(&txt->filename);
    string_append_string(&txt->filename, string_from_cstring(filename));
}
void text_prompt_filename(StringBuilder* sb) {
    string_clear(sb);
    printf("filename: ");
    string_scanln(sb);
}