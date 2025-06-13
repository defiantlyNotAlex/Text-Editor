#include "inputs.h"
#include <raylib.h>
#include "stringbuilder.h"

void inputs_get_inputs(Inputs* inputs, float dt) {
    for (isize i = 0; i < 384; i++) {
        inputs->down[i] = IsKeyDown(i);
        inputs->pressed_repeat[i] = inputs->pressed[i] = IsKeyPressed(i);
        inputs->released[i] = IsKeyReleased(i);

        if (inputs->down[i]) {
            inputs->down_time[i] += dt;
            if (inputs->down_time[i] > inputs->cooldown) {
                inputs->down_time[i] -= inputs->repeat_rate;
                inputs->pressed_repeat[i] = true;
            }
        } else {
            inputs->down_time[i] = 0.0f;
        }
    }
}