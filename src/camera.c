#include "camera.h"
#include <math.h>
MouseCursorPosition camera_mouse_pos(TextCamera* camera, Text* txt, Font font) {
    MouseCursorPosition mouse_pos = {0};
    float spacing = 1.0;
    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    Vector2 mpos = GetMousePosition();

    float padding = 10.0;

    Vector2 curr_pos = {.x = padding, .y = padding};

    if (camera->row < 0) camera->row = 0;
    if (camera->row > arrlist_count(txt->line_offsets)) camera->row = arrlist_count(txt->line_offsets);

    isize row = camera->row;
    isize col = 0;
    isize real_col = 0;

    for (isize i = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; i < gapbuf_count(&txt->gapbuf) && curr_pos.y + camera->scale * font.baseSize < screen_height - padding;) {
        Codepoint c = gapbuf_next_codepoint(&txt->gapbuf, &i);


        if (real_col >= 80 || curr_pos.x > screen_width - padding * 2) {
            real_col = 0;
            curr_pos.x = padding;
            curr_pos.y += font.baseSize * camera->scale;
        } else if (c == '\n') {
            row++;
            col = 0;
            real_col = 0;
            curr_pos.x = padding;
            curr_pos.y += font.baseSize * camera->scale;
        }
        if (c != '\n') {
            col++;
            real_col++;

            isize index = GetGlyphIndex(font, c);
            float width = 0;
            if (font.glyphs[index].advanceX == 0) width = font.recs[index].width * camera->scale;
            else width = font.glyphs[index].advanceX * camera->scale;
            width += spacing;
            
            if (CheckCollisionPointRec(mpos, (Rectangle){curr_pos.x, curr_pos.y, width, font.baseSize * camera->scale})) {
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
    float spacing = 1.0;
    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

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

    for (isize i = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; i < gapbuf_count(&txt->gapbuf) && curr_pos.y + camera->scale * font.baseSize < screen_height - padding;) {
        Codepoint c = gapbuf_next_codepoint(&txt->gapbuf, &i);

        if (real_col >= 80 || curr_pos.x > screen_width - padding * 2) {
            real_col = 0;
            curr_pos.x = padding;
            curr_pos.y += font.baseSize * camera->scale;
        } else if (c == '\n') {
            row++;
            col = 0;
            real_col = 0;
            curr_pos.x = padding;
            curr_pos.y += font.baseSize * camera->scale;
        }
        if (c != '\n') {
            col++;
            real_col++;

            isize index = GetGlyphIndex(font, c);
            float width = 0;
            if (font.glyphs[index].advanceX == 0) width = font.recs[index].width * camera->scale;
            else width = font.glyphs[index].advanceX * camera->scale;
            width += spacing;
            
            if (i > l && i <= r && txt->selected) {
                DrawRectangle(curr_pos.x, curr_pos.y, width, font.baseSize * camera->scale, GetColor(0x0000ffff));
            }
            curr_pos.x += width;
        }
    }
    row = camera->row;
    col = 0;
    real_col = 0;
    curr_pos = (Vector2){.x = padding, .y = padding};

    for (isize i = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; i < gapbuf_count(&txt->gapbuf) && curr_pos.y + camera->scale * font.baseSize < screen_height - padding;) {
        Codepoint c = gapbuf_next_codepoint(&txt->gapbuf, &i);
        
        if (row == txt->cursor_row && col == txt->cursor_col) {
            DrawRectangle(curr_pos.x, curr_pos.y, ceilf(camera->scale * 0.5) + 1, font.baseSize * camera->scale, BLUE);
        }
        if (real_col >= 80 || curr_pos.x > screen_width - padding * 2) {
            real_col = 0;

            curr_pos.x = padding;
            curr_pos.y += font.baseSize * camera->scale;

            if (c == '\n') {
                col = 0;
                row++;
            }
        } else if (c == '\n') {
            row++;
            col = 0;
            real_col = 0;

            curr_pos.x = padding;
            curr_pos.y += font.baseSize * camera->scale;
        }
        if (c != '\n') {
            col++;
            real_col++;

            isize index = GetGlyphIndex(font, c);
            float width = 0;
            if (font.glyphs[index].advanceX == 0) width = font.recs[index].width * camera->scale;
            else width = font.glyphs[index].advanceX * camera->scale;
            width += spacing;

            Color colour = BLACK;
            if (i > l && i <= r && txt->selected) {
                colour = GRAY;
            }
            if (c != U'\r') {
                DrawTextCodepoint(font, c, curr_pos, font.baseSize * camera->scale, colour);
            }
            curr_pos.x += width;
        }
    }
    if (row == txt->cursor_row && col == txt->cursor_col) {
        DrawRectangle(curr_pos.x, curr_pos.y, ceilf(camera->scale * 0.5) + 1, font.baseSize * camera->scale, BLUE);
    }
}