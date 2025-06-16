#include <raylib.h>
#include "short_types.h"
#include "text.h"

typedef struct MouseCursorPosition {
    CursorPosition pos;
    bool exists;
} MouseCursorPosition;

typedef struct TextCamera {
    isize row;

    float scale;
} TextCamera;

MouseCursorPosition camera_draw(TextCamera* camera, Text* txt, Font font);