#include "camera.h"
#include <math.h>

TextCamera camera_default() {
    return (TextCamera) {
        .max_cols = 80,
        .padding = 5.0,
        .spacing = 1.0,

        .left_margin = 20.0,
        .bottom_margin = 10.0,
    };
}
MouseCursorPosition camera_mouse_pos(TextCamera* camera, Text* txt, Font font) {
    MouseCursorPosition mouse_pos = {0};
    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    Vector2 mpos = GetMousePosition();

    Vector2 curr_pos = {.x = camera->padding + camera->left_margin, .y = camera->padding};
    float bottom = screen_height - camera->padding - camera->bottom_margin;

    if (camera->row < 0) camera->row = 0;
    if (camera->row > arrlist_count(txt->line_offsets)) camera->row = arrlist_count(txt->line_offsets);

    isize row = camera->row;
    isize col = 0;
    isize real_col = 0;

    for (isize i = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; i < gapbuf_count(&txt->gapbuf) && curr_pos.y < bottom;) {
        Codepoint c = gapbuf_next_codepoint(&txt->gapbuf, &i);

        if (real_col >= camera->max_cols || curr_pos.x > screen_width - camera->padding) {
            real_col = 0;
            curr_pos.x = camera->padding + camera->left_margin;
            curr_pos.y += font.baseSize;
            if (curr_pos.y > bottom) break;
        } else if (c == '\n') {
            row++;
            col = 0;
            real_col = 0;
            curr_pos.x = camera->padding + camera->left_margin;
            curr_pos.y += font.baseSize ;
        }
        if (c != '\n') {
            col++;
            real_col++;

            isize index = GetGlyphIndex(font, c);
            float width = 0;
            if (font.glyphs[index].advanceX == 0) width = font.recs[index].width ;
            else width = font.glyphs[index].advanceX;
            width += camera->spacing;
            
            if (CheckCollisionPointRec(mpos, (Rectangle){curr_pos.x, curr_pos.y, width, font.baseSize})) {
                if (curr_pos.x + width / 2 > mpos.x) {
                    mouse_pos.pos.col = col - 1;
                } else {
                    mouse_pos.pos.col = col;
                }
                mouse_pos.exists = true;
                mouse_pos.pos.row = row;

                return mouse_pos;
            }
            curr_pos.x += width;
        }
    }
    return mouse_pos;
}
void camera_draw(TextCamera* camera, Text* txt, Font font) {
    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    Vector2 curr_pos = {.x = camera->padding + camera->left_margin, .y = camera->padding};

    if (camera->row < 0) camera->row = 0;
    if (camera->row > arrlist_count(txt->line_offsets)) camera->row = arrlist_count(txt->line_offsets);

    float bottom = screen_height - camera->padding - camera->bottom_margin;

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

    for (isize i = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; i < gapbuf_count(&txt->gapbuf) && curr_pos.y < bottom - font.baseSize;) {
        Codepoint c = gapbuf_next_codepoint(&txt->gapbuf, &i);

        if (real_col >= camera->max_cols || curr_pos.x > screen_width - camera->padding) {
            real_col = 0;
            curr_pos.x = camera->padding + camera->left_margin;
            curr_pos.y += font.baseSize;

            if (curr_pos.y > bottom - font.baseSize) break;
        } else if (c == '\n') {
            row++;
            col = 0;
            real_col = 0;
            curr_pos.x = camera->padding + camera->left_margin;
            curr_pos.y += font.baseSize ;
        }
        if (c != '\n') {
            col++;
            real_col++;

            isize index = GetGlyphIndex(font, c);
            float width = 0;
            if (font.glyphs[index].advanceX == 0) width = font.recs[index].width ;
            else width = font.glyphs[index].advanceX ;
            width += camera->spacing;
            
            if (i > l && i <= r && txt->selected) {
                DrawRectangle(curr_pos.x, curr_pos.y, width, font.baseSize , GetColor(0x0000ffff));
            }
            curr_pos.x += width;
        }
    }
    row = camera->row;
    col = 0;
    real_col = 0;
    curr_pos = (Vector2){.x = camera->padding + camera->left_margin, .y = camera->padding};

    for (isize i = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; i < gapbuf_count(&txt->gapbuf) && curr_pos.y < bottom- font.baseSize;) {
        Codepoint c = gapbuf_next_codepoint(&txt->gapbuf, &i);
        if (col == 0) {
            DrawTextEx(font, TextFormat("%d", row + 1), (Vector2){.x = camera->padding, .y = curr_pos.y}, font.baseSize, camera->spacing, BLACK);
        }

        if (row == txt->cursor_row && col == txt->cursor_col) {
            DrawRectangle(curr_pos.x, curr_pos.y, ceilf(0.5) + 1, font.baseSize , BLUE);
        }
        if (real_col >= camera->max_cols || curr_pos.x + font.baseSize > screen_width - camera->padding) {
            real_col = 0;

            curr_pos.x = camera->padding + camera->left_margin;
            curr_pos.y += font.baseSize ;

            if (c == '\n') {
                col = 0;
                row++;
            }
            if (curr_pos.y > bottom - font.baseSize) break;
        } else if (c == '\n') {
            row++;
            col = 0;
            real_col = 0;

            curr_pos.x = camera->padding + camera->left_margin;
            curr_pos.y += font.baseSize ;
        }



        if (c != '\n') {
            col++;
            real_col++;

            isize index = GetGlyphIndex(font, c);
            float width = 0;
            if (font.glyphs[index].advanceX == 0) width = font.recs[index].width ;
            else width = font.glyphs[index].advanceX ;
            width += camera->spacing;

            Color colour = BLACK;
            if (i > l && i <= r && txt->selected) {
                colour = GRAY;
            }
            if (c != U'\r') {
                DrawTextCodepoint(font, c, curr_pos, font.baseSize , colour);
            }
            curr_pos.x += width;
        }
    }
    if (curr_pos.y < bottom - font.baseSize) {
        if (col == 0) {
            DrawTextEx(font, TextFormat("%d", row + 1), (Vector2){.x = camera->padding, .y = curr_pos.y}, font.baseSize, camera->spacing, BLACK);
        }
        if (row == txt->cursor_row && col == txt->cursor_col) {
            DrawRectangle(curr_pos.x, curr_pos.y, ceilf(0.5) + 1, font.baseSize , BLUE);
        }
    }
}