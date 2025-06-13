#ifndef INPUTS_H_
#define INPUTS_H_

#include "short_types.h"

typedef struct Inputs {
    bool pressed[384];
    bool pressed_repeat[384];

    bool released[384];
    bool down[384];

    float down_time[384];

    float cooldown;
    float repeat_rate;
} Inputs;

void inputs_get_inputs(Inputs* inputs, float dt);

#endif //INPUTS_H_