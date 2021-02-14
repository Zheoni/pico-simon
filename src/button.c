#include "button.h"
#include "hardware/gpio.h"

void button_init(uint button_pin, enum button_pull pull, bool pressed_state, button* b) {
    b->pin = button_pin;
    gpio_init(button_pin);
    gpio_set_dir(button_pin, GPIO_IN);
    gpio_set_pulls(button_pin, pull == BUTTON_PULL_UP, pull == BUTTON_PULL_DOWN);

    b->pressed_state = pressed_state;
    button_prepare_for_loop(b);
    b->debounce_time = 20000;
}

bool button_get(button* b) {
    return gpio_get(b->pin);
}

void button_prepare_for_loop(button* b) {
    b->last_state = gpio_get(b->pin);
    b->last_flicker = b->last_state;
    b->timestamp = make_timeout_time_us(b->debounce_time);
}

button_change_t button_change_steady(button* b) {
    bool current_state = gpio_get(b->pin);

    if (current_state == b->last_state) {
        return BUTTON_NONE;
    }

    if (current_state != b->last_flicker) {
        b->timestamp = make_timeout_time_us(b->debounce_time);
        b->last_flicker = current_state;
    }

    if (time_reached(b->timestamp)) {
        b->last_state = current_state;
        if (b->last_state == b->pressed_state) {
            return BUTTON_PRESS;
        } else {
            return BUTTON_RELEASE;
        }
    }
    return BUTTON_NONE;
}
