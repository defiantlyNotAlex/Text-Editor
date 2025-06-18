#include <raylib.h>
#include "short_types.h"
#include "text.h"

typedef struct MouseCursorPosition {
    CursorPosition pos;
    bool exists;
} MouseCursorPosition;

typedef struct TextCamera {
    isize row;

    float spacing;
    float padding;
    int max_cols;
} TextCamera;

TextCamera camera_default();
MouseCursorPosition camera_mouse_pos(TextCamera* camera, Text* txt, Font font);
void camera_draw(TextCamera* camera, Text* txt, Font font);