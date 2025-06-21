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

Codepoint camera_next_char(TextCamera* camera, Text* txt, Font font, CameraPosition* pos) {
    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    float bottom = screen_height - camera->padding - camera->bottom_margin;

    Codepoint c = gapbuf_next_codepoint(&txt->gapbuf, &pos->index);

    if (c != '\n') {
        pos->col++;
        pos->true_col++;

        isize glyph_index = GetGlyphIndex(font, c);
        pos->width = 0;
        if (font.glyphs[glyph_index].advanceX == 0) pos->width = font.recs[glyph_index].width ;
        else pos->width = font.glyphs[glyph_index].advanceX;
        pos->width += camera->spacing;
    }

    if (pos->true_col >= camera->max_cols || pos->position.x + pos->width > screen_width - camera->padding || c == '\n') {
        pos->true_col = 0;
        pos->true_line++;

        pos->position.x = camera->padding + camera->left_margin;
        pos->position.y += font.baseSize ;
        
        if (c == '\n') {
            pos->line++;
            pos->col = 0;
        }
        if (pos->position.y > bottom) return c;
    }

    return c;
}
MouseCursorPosition camera_mouse_pos(TextCamera* camera, Text* txt, Font font) {
    MouseCursorPosition mouse_pos = {0};
    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    Vector2 mpos = GetMousePosition();

    float bottom = screen_height - camera->padding - camera->bottom_margin;

    if (camera->row < 0) camera->row = 0;
    if (camera->row > arrlist_count(txt->line_offsets)) camera->row = arrlist_count(txt->line_offsets);

    CameraPosition pos = {
        .position = {camera->padding + camera->left_margin, .y = camera->padding},
        .line = camera->row,
    };
    isize line = pos.true_line;
    isize old_line = pos.line;
    isize old_col = pos.col;
    Vector2 old_pos = pos.position;
    for (pos.index = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; pos.index < gapbuf_count(&txt->gapbuf) && pos.position.y + font.baseSize < bottom;) {
        line = pos.true_line;
        old_line = pos.line;
        old_col = pos.col;
        old_pos = pos.position;
        Codepoint c = camera_next_char(camera, txt, font, &pos);
        
        Rectangle char_rect = {pos.position.x, pos.position.y, pos.width, font.baseSize};
        if (CheckCollisionPointRec(mpos, char_rect)) {
            #ifdef DEBUG
            DrawRectangleRec(char_rect, GetColor(0xff0000a0));
            #endif
            if (pos.position.x + pos.width / 2 > mpos.x) {
                mouse_pos.pos.col = pos.col - 1;
            } else {
                mouse_pos.pos.col = pos.col;
            }
            mouse_pos.exists = true;
            mouse_pos.pos.line = pos.line;
            
            return mouse_pos;
        }
        Rectangle line_rect = {camera->padding + camera->left_margin, old_pos.y, screen_width - line_rect.x, font.baseSize};
        if (line != pos.true_line && CheckCollisionPointRec(mpos, line_rect)) {
            #ifdef DEBUG
                DrawRectangleRec(line_rect, GetColor(0xff0000a0));
            #endif
            mouse_pos.exists = true;
            mouse_pos.pos.line = old_line;
            mouse_pos.pos.col = old_col;
            return mouse_pos;
        }
        if (c != '\r' && c != '\n') {
            pos.position.x += pos.width;
        }
    }
    Rectangle line_rect = {camera->padding + camera->left_margin, pos.position.y, screen_width - line_rect.x, font.baseSize};
    if (CheckCollisionPointRec(mpos, line_rect)) {
        #ifdef DEBUG
            DrawRectangleRec(line_rect, GetColor(0xff0000a0));
        #endif
        mouse_pos.exists = true;
        mouse_pos.pos.line = pos.line;
        mouse_pos.pos.col = pos.col;
        return mouse_pos;
    }
    return mouse_pos;
}
void camera_draw(TextCamera* camera, Text* txt, Font font) {
    //float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();
    
    
    float bottom = screen_height - camera->padding - camera->bottom_margin;
    
    if (camera->row < 0) camera->row = 0;
    if (camera->row > arrlist_count(txt->line_offsets)) camera->row = arrlist_count(txt->line_offsets);

    isize l, r;
    if (txt->selection_begin < txt->selection_end) {
        l = txt->selection_begin;
        r = txt->selection_end;
    } else {
        l = txt->selection_end;
        r = txt->selection_begin;
    }

    CameraPosition pos = {
        .position = {camera->padding + camera->left_margin, .y = camera->padding},
        .line = camera->row,
    };

    for (pos.index = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; pos.index < gapbuf_count(&txt->gapbuf) && pos.position.y + font.baseSize < bottom;) {
        Codepoint c = camera_next_char(camera, txt, font, &pos);
    
        if (c != '\r' && c != '\n') {
            if (pos.index > l && pos.index <= r && txt->selected) {
                DrawRectangle(pos.position.x, pos.position.y, pos.width, font.baseSize, highlight_colour);
            }
            pos.position.x += pos.width;
        }
    }

    CameraPosition pos2 = {
        .position = {camera->padding + camera->left_margin, .y = camera->padding},
        .line = camera->row,
    };

    for (pos2.index = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; pos2.index < gapbuf_count(&txt->gapbuf) && pos2.position.y + font.baseSize < bottom;) {
        if (pos2.col == 0) {
            DrawTextEx(font, TextFormat("%d", pos2.line + 1), (Vector2){.x = camera->padding, .y = pos2.position.y}, font.baseSize, camera->spacing, text_colour);
        }
        Codepoint c = camera_next_char(camera, txt, font, &pos2);
        if (c != '\r' && c != '\n') {
            pos.position.x += pos.width;
        }
    }

    CameraPosition pos3 = {
        .position = {camera->padding + camera->left_margin, .y = camera->padding},
        .line = camera->row,
    };

    

    for (pos3.index = camera->row != 0 ? txt->line_offsets[camera->row - 1] : 0; pos3.index < gapbuf_count(&txt->gapbuf) && pos3.position.y + font.baseSize < bottom;) {
        if (pos3.line == txt->cursor_line && pos3.col == txt->cursor_col) {
            DrawRectangle(pos3.position.x, pos3.position.y, 2, font.baseSize, cursor_colour);
        }
        
        Codepoint c = camera_next_char(camera, txt, font, &pos3);
    
        if (c != '\r' && c != '\n') {
            DrawTextCodepoint(font, c, pos3.position, font.baseSize, text_colour);
            pos3.position.x += pos3.width;
        }
    }

    
    if (pos3.position.y < bottom - font.baseSize) {
        if (pos3.col == 0) {
            DrawTextEx(font, TextFormat("%d", pos3.line + 1), (Vector2){.x = camera->padding, .y = pos3.position.y}, font.baseSize, camera->spacing, text_colour);
        }
        if (pos3.line == txt->cursor_line && pos3.col == txt->cursor_col) {
            DrawRectangle(pos3.position.x, pos3.position.y, 2, font.baseSize , cursor_colour);
        }
    }
}