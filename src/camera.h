#include <raylib.h>
#include "short_types.h"
#include "text.h"

typedef struct MouseCursorPosition {
    CursorPosition pos;
    bool exists;
} MouseCursorPosition;

typedef struct TextCamera {
    isize row;

    float padding;
    float left_margin;
    float bottom_margin;

    float spacing;
    int max_cols;
} TextCamera;

typedef struct CameraPosition {
    isize line, col;
    isize true_line, true_col;

    isize index;
    Vector2 position;
    float width;
} CameraPosition;

TextCamera camera_default();

Codepoint camera_next_char(TextCamera* camera, Text* txt, Font font, CameraPosition* pos);

MouseCursorPosition camera_mouse_pos(TextCamera* camera, Text* txt, Font font);
void camera_draw(TextCamera* camera, Text* txt, Font font);