#include <raylib.h>
#include <raymath.h>
#define STRINGBUILDER_IMPLEMENTATION
#include "stringbuilder.h"
#define ARENA_IMPLEMENTATION
#include "arena.h"
#define ARRAYLIST_IMPLEMENTATION
#include "arraylist.h"

#include "short_types.h"

#define GAPBUFFER_IMPLEMENTATION
#include "gapbuffer.h"

#include "text.h"
#include "undo.h"
#include "inputs.h"
#include "camera.h"


int keys[] = {KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_ZERO, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE, KEY_EQUAL, KEY_MINUS, KEY_COMMA, KEY_PERIOD, KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_SLASH, KEY_GRAVE, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_TAB, KEY_SPACE, KEY_ENTER};

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

bool still_word(Codepoint c) {
    if (string_is_ascii_alpha(c) || string_is_digit(c, NULL) || c == '_') {
        return false;
    }
    return true;
}

bool is_alpha_numeric(Codepoint c) {
    if (string_is_ascii_alpha(c) || string_is_digit(c, NULL) || c == '_') {
        return true;
    }
    return false;
}

int main(i32 argc, char** argv) {
    SetTraceLogLevel(LOG_ERROR);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Text-Editor");
    SetTargetFPS(60);
    
    Text txt = {0};
    TextCamera camera = {.scale = 1.0};
    Inputs inputs = {.cooldown = 0.5, .repeat_rate = 0.05};
    Font font = LoadFontEx("fonts/ComicMono.ttf", 20, NULL, 0);
    
    if (argc > 1) {
        text_load_file(&txt, argv[1]);
    }

    bool alpha_num_streak = false;
    bool space_streak = false;
    bool delete_streak = false;

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();
        inputs_get_inputs(&inputs, dt);

        
        bool cntrl = inputs.down[KEY_LEFT_CONTROL] || inputs.down[KEY_RIGHT_CONTROL];
        bool shift = inputs.down[KEY_LEFT_SHIFT] || inputs.down[KEY_RIGHT_SHIFT];

        if (cntrl && inputs.pressed[KEY_S]) {
            text_save_file(&txt);
        } else if (cntrl && inputs.pressed[KEY_L]) {
            StringBuilder sb = {0};
            text_prompt_filename(&sb);
            text_load_file(&txt, sb.data);
            reset_command(&txt.commands);
            string_free(&sb);

            alpha_num_streak = false;
            space_streak = false;
            delete_streak = false;
        } else if (cntrl && inputs.pressed_repeat[KEY_V]) {
            text_begin_command(&txt);

            const char* str = GetClipboardText();

            text_cursor_insert(&txt, string_from_cstring(str));
            text_end_command(&txt);
        } else if (cntrl && inputs.pressed[KEY_X]) {
            text_begin_command(&txt);
            text_copy_and_delete_selection_to_clipboard(&txt);
            text_end_command(&txt);
        } else if (cntrl && inputs.pressed[KEY_C]) {
            // ctrl c
            text_copy_selection_to_clipboard(&txt);
        } else if (cntrl && inputs.pressed_repeat[KEY_Z]) {
            alpha_num_streak = false;
            space_streak = false;
            delete_streak = false;
            text_undo(&txt);
        } else if (cntrl && inputs.pressed_repeat[KEY_Y]) {
            text_redo(&txt);
        }

        text_cursor_update_position(&txt);
        float mousewheel_movement = GetMouseWheelMove();
        if (mousewheel_movement != 0) {
            camera.row -= mousewheel_movement;
        }

        bool cursor_moved = false;

        if (inputs.pressed_repeat[KEY_LEFT]) {
            if (txt.selected && !shift && txt.selection_begin != txt.selection_end) { 
                text_cursor_move_to_selected(&txt, false);
                cursor_moved = true;
            } else if (cntrl) {
                text_cursor_move_until(&txt, false, still_word);
                cursor_moved = true;
            } else {
                text_cursor_move_codepoints(&txt, -1);
                cursor_moved = true;
            }
        } else if (inputs.pressed_repeat[KEY_RIGHT]) {
            if (txt.selected && !shift && txt.selection_begin != txt.selection_end) { 
                text_cursor_move_to_selected(&txt, true);
                cursor_moved = true;
            } else if (cntrl) {
                text_cursor_move_until(&txt, true, still_word);
                cursor_moved = true;
            } else {
                text_cursor_move_codepoints(&txt, 1);
                cursor_moved = true;
            }
        } else if (inputs.pressed_repeat[KEY_UP]) {
            if (cntrl) {
                camera.row--;
            } else if (txt.selected && !shift && txt.selection_begin != txt.selection_end) {
                text_cursor_move_to_selected(&txt, false);
                cursor_moved = true;
            } else {
                text_cursor_moveto(&txt, txt.cursor_col, txt.cursor_row - 1);
                cursor_moved = true;
            }
        } else if (inputs.pressed_repeat[KEY_DOWN]) {
            if (cntrl) {
                camera.row++;
            } else if (txt.selected && !shift && txt.selection_begin != txt.selection_end) {
                text_cursor_move_to_selected(&txt, true);
                cursor_moved = true;
            } else {
                text_cursor_moveto(&txt, txt.cursor_col, txt.cursor_row + 1);
                cursor_moved = true;
            }
        } else if (inputs.pressed_repeat[KEY_HOME]) {
            text_cursor_moveto(&txt, 0, txt.cursor_row); cursor_moved = true; 
        } else if (inputs.pressed_repeat[KEY_END]) {
            text_cursor_moveto(&txt, ISIZE_MAX, txt.cursor_row); cursor_moved = true; 
        } else if (inputs.pressed_repeat[KEY_PAGE_UP]) {
            if (cntrl) {
                camera.row -= 20;
            } else {
                text_cursor_moveto(&txt, txt.cursor_col, txt.cursor_row - 20);
                cursor_moved = true;
            }
        } else if (inputs.pressed_repeat[KEY_PAGE_DOWN]) {
            if (cntrl) {
                camera.row += 20;
            } else {
                text_cursor_moveto(&txt, txt.cursor_col, txt.cursor_row + 20);
                cursor_moved = true;
            } 
        }

        if (shift) {
            text_select_end(&txt);
        }
        text_update_line_offsets(&txt);
        text_cursor_update_position(&txt);

        int lines_fit_on_screen = (GetScreenHeight() - 20) / (font.baseSize * camera.scale) - 2;
        if (cursor_moved) {
            if (txt.cursor_row < camera.row) {
                camera.row = txt.cursor_row;
            } else if (txt.cursor_row > camera.row + lines_fit_on_screen) {
                camera.row = txt.cursor_row - lines_fit_on_screen;
            }

            if (!shift) {
                txt.selected = false;
            }
        }

        if (inputs.pressed_repeat[KEY_BACKSPACE] || inputs.pressed_repeat[KEY_DELETE]) {
            if (!delete_streak) {
                text_begin_command(&txt);
            }
            
            if (inputs.pressed_repeat[KEY_BACKSPACE]) {
                text_cursor_remove_before(&txt, 1);
            } else if (inputs.pressed_repeat[KEY_DELETE]) {
                text_cursor_remove_after(&txt, 1);
            }

            delete_streak = true;
            alpha_num_streak = false;
            space_streak = false;
        }
        
        if (!txt.selected && shift) {
            text_select_begin(&txt);
        }

        for (isize i = 0; i < countof(keys); i++) {
            int key = keys[i];
            if (inputs.pressed_repeat[key]) {
                char* s = keycode_to_char(key, shift);            
                if (s != NULL && !cntrl) {
                    if (alpha_num_streak && (is_alpha_numeric(*s))) {}
                    else if (space_streak && (string_is_ascii_whitespace(*s))) {}
                    else text_begin_command(&txt);

                    text_cursor_insert(&txt, string_from_cstring(s));

                    alpha_num_streak = false;
                    space_streak = false;
                    delete_streak = false;

                    if (is_alpha_numeric(*s)) alpha_num_streak = true;
                    if (string_is_ascii_whitespace(*s)) space_streak = true;
                }
            }
        }
        
        text_update_line_offsets(&txt);
        text_cursor_update_position(&txt);

        BeginDrawing();
        MouseCursorPosition mouse_pos = camera_mouse_pos(&camera, &txt, font);
        camera_draw(&camera, &txt, font);
        if (mouse_pos.exists && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            text_cursor_moveto(&txt, mouse_pos.pos.col, mouse_pos.pos.row);
            if (shift) {
                text_select_end(&txt);
            } else {
                text_select_begin(&txt);
            }
        }
        if (mouse_pos.exists && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            text_cursor_moveto(&txt, mouse_pos.pos.col, mouse_pos.pos.row);
            text_select_end(&txt);
        }

        DrawText(TextFormat("line: %ld, col: %ld", txt.cursor_row, txt.cursor_col), 1000, 0, 10, BLACK);
    
        ClearBackground(WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}