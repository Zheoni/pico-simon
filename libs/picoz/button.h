#ifndef BUTTON_H
#define BUTTON_H

#include "pico/types.h"

typedef struct {
    uint pin;
    bool pressed_state;
    bool last_state;
    uint pio_idx;
    uint sm;
} button;

enum button_pull {
    BUTTON_PULL_DOWN,
    BUTTON_PULL_UP,
};

typedef enum {
    BUTTON_NONE,
    BUTTON_PRESS,
    BUTTON_RELEASE,
} button_change_t;

bool button_init(uint button_pin, enum button_pull pull, bool pressed_state,
                 button* b);

void button_deinit(button* b);

bool button_set_debounce_time_us(button* b, uint32_t debounce_time_us);

bool button_get(button* b);

void button_prepare_for_loop(button* b);

button_change_t button_change_steady(button* b);

#endif /* end of include guard: BUTTON_H */
