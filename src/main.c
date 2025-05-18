#include <raylib.h>
#include <raymath.h>
#define ARENA_IMPLEMENTATION
#include "arena.h"
#define ARRAYLIST_IMPLEMENTATION
#include "arraylist.h"
#define STRINGBUILDER_IMPLEMENTATION
#include "stringbuilder.h"

#include "short_types.h"

#define GAPBUFFER_IMPLEMENTATION
#include "gapbuffer.h"

#include "text.h"


char* keycode_to_char(KeyboardKey key, bool upper) {
    switch (key)
    {
        case KEY_A: return upper ? "A" : "a";
        case KEY_B: return upper ? "B" : "b";
        case KEY_C: return upper ? "C" : "c";
        case KEY_D: return upper ? "D" : "d";
        case KEY_E: return upper ? "E" : "e";
        case KEY_F: return upper ? "F" : "f";
        case KEY_G: return upper ? "G" : "g";
        case KEY_H: return upper ? "H" : "h";
        case KEY_I: return upper ? "I" : "i";
        case KEY_J: return upper ? "J" : "j";
        case KEY_K: return upper ? "K" : "k";
        case KEY_L: return upper ? "L" : "l";
        case KEY_M: return upper ? "M" : "m";
        case KEY_N: return upper ? "N" : "n";
        case KEY_O: return upper ? "O" : "o";
        case KEY_P: return upper ? "P" : "p";
        case KEY_Q: return upper ? "Q" : "q";
        case KEY_R: return upper ? "R" : "r";
        case KEY_S: return upper ? "S" : "s";
        case KEY_T: return upper ? "T" : "t";
        case KEY_U: return upper ? "U" : "u";
        case KEY_V: return upper ? "V" : "v";
        case KEY_W: return upper ? "W" : "w";
        case KEY_X: return upper ? "X" : "x";
        case KEY_Y: return upper ? "Y" : "y";
        case KEY_Z: return upper ? "Z" : "z";

        case KEY_ONE:   return upper ? "!" : "1";
        case KEY_TWO:   return upper ? "@" : "2";
        case KEY_THREE: return upper ? "#" : "3";
        case KEY_FOUR:  return upper ? "$" : "4";
        case KEY_FIVE:  return upper ? "%" : "5";
        case KEY_SIX:   return upper ? "^" : "6";
        case KEY_SEVEN: return upper ? "&" : "7";
        case KEY_EIGHT: return upper ? "*" : "8";
        case KEY_NINE:  return upper ? "(" : "9";
        case KEY_ZERO:  return upper ? ")" : "0";

        case KEY_EQUAL:  return upper ? "+" : "=";
        case KEY_MINUS:  return upper ? "_" : "-";
        case KEY_COMMA: return upper ? "<" : ",";
        case KEY_PERIOD: return upper ? ">" : ".";

        case KEY_LEFT_BRACKET: return upper ? "{" : "[";
        case KEY_RIGHT_BRACKET: return upper ? "}" : "]";
        case KEY_BACKSLASH: return upper ? "|" : "\\";
        case KEY_SLASH: return upper ? "?" : "/";
        case KEY_GRAVE: return upper ? "~" : "`";
        case KEY_SEMICOLON: return upper ? ":" : ";";
        case KEY_APOSTROPHE: return upper ? "\"" : "\"";
        case KEY_TAB: return "    ";
        case KEY_SPACE: return " ";
        case KEY_ENTER: return "\n";

        default: return NULL; 
    }
}

typedef struct TextCamera {
    isize row;

    float scale;
} TextCamera;

void text_draw(TextCamera* camera, Text* txt, Font font) {
    float spacing = 1.0;
    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    GapBufSlice strings = gapbuf_getstrings(&txt->gapbuf);

    float padding = 10.0;

    Vector2 curr_pos = {.x = padding, .y = padding};

    if (camera->row < 0) camera->row = 0;
    if (camera->row > arrlist_count(txt->line_offsets)) camera->row = arrlist_count(txt->line_offsets);

    isize row = camera->row;
    isize col = 0;
    isize real_col = 0;

    isize l, r;
    if (txt->selection_begin < txt->selection_end) {
        l = txt->selection_begin;
        r = txt->selection_end;
    } else {
        l = txt->selection_end;
        r = txt->selection_begin;
    }

    for (isize i = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; i < strings.l.count + strings.r.count && curr_pos.y + camera->scale * font.baseSize < screen_height - padding;) {
        Codepoint c = '\0';
        if (i < strings.l.count) {
            c = string_next_codepoint(strings.l, &i);
        } else {
            isize j = i - strings.l.count;
            c = string_next_codepoint(strings.r, &j);
            i = j + strings.l.count;
        }

        
        if (row == txt->cursor_row && col == txt->cursor_col) {
            DrawRectangle(curr_pos.x, curr_pos.y, ceilf(camera->scale * 0.5) + 1, font.baseSize * camera->scale, BLUE);
        }
        if (real_col > 80 || curr_pos.x > screen_width - padding * 2) {
            real_col = 0;
            curr_pos.x = padding;
            curr_pos.y += font.baseSize * camera->scale;
        }
        if (c == U'\r') continue;
        if (c == U'\n') {
            row++;
            col = 0;
            real_col = 0;
            curr_pos.x = padding;
            curr_pos.y += font.baseSize * camera->scale;
        } else {
            col++;
            real_col++;
            isize index = GetGlyphIndex(font, c);
            float width = 0;
            if (font.glyphs[index].advanceX == 0) width = ((float)font.recs[index].width*camera->scale);
            else width  = ((float)font.glyphs[index].advanceX*camera->scale);
            width += spacing;
            if (i > l && i <= r && txt->selected) {
                DrawRectangle(curr_pos.x, curr_pos.y, width, font.baseSize * camera->scale, GetColor(0x0000ffff));
                DrawTextCodepoint(font, c, curr_pos, 10 * camera->scale, WHITE);
            } else {
                DrawTextCodepoint(font, c, curr_pos, 10 * camera->scale, BLACK);
            }
            curr_pos.x += width;
        }
    }
    if (row == txt->cursor_row && col == txt->cursor_col) {
        DrawRectangle(curr_pos.x, curr_pos.y, ceilf(camera->scale * 0.5) + 1, font.baseSize * camera->scale, BLUE);
    }
}

void load_file(Text* txt, const char* filename) {
    gapbuf_read_entire_file(&txt->gapbuf, filename);
    text_update_line_offsets(txt);
    text_cursor_update_position(txt);
}
void save_file(Text* txt, const char* filename) {
    gapbuf_write_entire_file(&txt->gapbuf, filename);
}


bool still_word(Codepoint c) {
    if (string_is_ascii_alpha(c) || string_is_digit(c, NULL) || c == '_') {
        return false;
    }
    return true;
}


int main(i32 argc, char** argv) {

    
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(1280, 720, "Text-Editor");
    SetTargetFPS(60);
    
    TextCamera camera = {.scale = 2.0};
    Text txt = {0};
    Font font = GetFontDefault();
    
    if (argc > 1) {
        load_file(&txt, argv[1]);
    }

    while(!WindowShouldClose()) {

        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
            StringBuilder sb = {0};
            printf("filename: ");
            string_scanln(&sb);
            save_file(&txt, sb.data);
            string_free(&sb);
        } else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L)) {
            StringBuilder sb = {0};
            printf("filename: ");
            string_scanln(&sb);
            load_file(&txt, sb.data);
            string_free(&sb);
        }
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
            if (txt.selected)
                text_delete_selection(&txt);
            const char* str = GetClipboardText();
            text_cursor_insert(&txt, str, strlen(str));
            text_update_line_offsets(&txt);
            text_cursor_update_position(&txt);
        }
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_X)) {
            text_copy_selection_to_clipboard(&txt);
            text_delete_selection(&txt);
        }
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C)) {
            text_copy_selection_to_clipboard(&txt);
        }

        KeyboardKey key = 0;
        if (!txt.selected && IsKeyDown(KEY_LEFT_SHIFT)) {
            text_select_begin(&txt);
        }
        do {
            key = GetKeyPressed();
            char* s = keycode_to_char(key, IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
    
            text_cursor_update_position(&txt);
            switch (key) {
                case KEY_LEFT: 
                case KEY_RIGHT:
                case KEY_UP: 
                case KEY_DOWN: 
                case KEY_HOME: 
                case KEY_END:
                if (!IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL)) {
                    txt.selected = false;
                }
                break;
                default: break;
            }
            switch (key) {
                case KEY_LEFT: if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    text_cursor_move_until(&txt, false, still_word);
                } else {
                    text_cursor_move_codepoints(&txt, -1);
                } break;
                    
                case KEY_RIGHT: if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    text_cursor_move_until(&txt, true, still_word);
                } else {
                    text_cursor_move_codepoints(&txt, 1);
                } break;

                case KEY_UP: if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    camera.row--;
                } else {
                    text_cursor_moveto(&txt, txt.cursor_col, txt.cursor_row - 1);
                } break;
                case KEY_DOWN: if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    camera.row++;
                } else {
                    text_cursor_moveto(&txt, txt.cursor_col, txt.cursor_row + 1);
                } break;

                case KEY_HOME: text_cursor_moveto(&txt, 0, txt.cursor_row); break;
                case KEY_END: text_cursor_moveto(&txt, ISIZE_MAX, txt.cursor_row); break;

                case KEY_PAGE_UP: if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    camera.row -= 20;
                } else {
                    text_cursor_moveto(&txt, txt.cursor_col, txt.cursor_row - 20); break;
                } break;
                case KEY_PAGE_DOWN: if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    camera.row += 20;
                } else {
                    text_cursor_moveto(&txt, txt.cursor_col, txt.cursor_row + 20); break;
                } break;
            
                default: break;
            }
            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                text_select_end(&txt);
            }
            
            text_cursor_update_position(&txt);
            
            if (txt.selected && (key == KEY_BACKSPACE || key == KEY_DELETE)) {
                text_delete_selection(&txt);
            } else if (key == KEY_BACKSPACE) {
                text_cursor_remove_before(&txt, 1);
            } else if (key == KEY_DELETE) {
                text_cursor_remove_after(&txt, 1);
            }
    
            if (s != NULL && !IsKeyDown(KEY_LEFT_CONTROL)) {
                if (txt.selected) text_delete_selection(&txt);
                text_cursor_insert(&txt, s, strlen(s));
                
            }
        } while(key != '\0');
        
        text_update_line_offsets(&txt);
        text_cursor_update_position(&txt);

        BeginDrawing();
        text_draw(&camera, &txt, font);
        DrawText(TextFormat("line: %ld, col: %ld", txt.cursor_row, txt.cursor_col), 1000, 0, 10, BLACK);
    
        ClearBackground(WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}