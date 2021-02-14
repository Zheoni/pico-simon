#ifndef BUTTON_H
#define BUTTON_H

#include "pico/time.h"

typedef struct {
    uint pin;
    absolute_time_t timestamp;
    uint32_t debounce_time;
    bool last_state;
    bool last_flicker;
    bool pressed_state;
} button;

enum button_pull {
    BUTTON_PULL_DOWN,
    BUTTON_PULL_UP,
};

typedef enum {
    BUTTON_NONE=0,
    BUTTON_PRESS,
    BUTTON_RELEASE,
} button_change_t;

void button_init(uint button_pin, enum button_pull pull, bool pressed_state, button* b);

inline void button_set_debounce_time(button* b, uint32_t ms) {
    b->debounce_time = ms * 1000;
}

bool button_get(button* b);

void button_prepare_for_loop(button* b);

button_change_t button_change_steady(button* b);

#endif /* end of include guard: BUTTON_H */
