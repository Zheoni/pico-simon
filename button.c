#include "button.h"
#include "hardware/gpio.h"

void button_init(uint button_pin, enum button_pull pull, bool pressed_state, button* b) {
    b->pin = button_pin;
    gpio_init(button_pin);
    gpio_set_dir(button_pin, GPIO_IN);
    gpio_set_pulls(button_pin, pull == BUTTON_PULL_UP, pull == BUTTON_PULL_DOWN);

    b->pressed_state = pressed_state;

    b->last_state = gpio_get(button_pin);
    b->last_flicker = b->last_state;
    b->timestamp = get_absolute_time();
    b->debounce_time = 20000;
}

button_change_t button_change_steady(button* b) {
    bool current_state = gpio_get(b->pin);

    if (current_state == b->last_state) {
        return BUTTON_NONE;
    }

    if (current_state != b->last_flicker) {
        b->timestamp = get_absolute_time();
        b->last_flicker = current_state;
    }

    if (absolute_time_diff_us(b->timestamp, get_absolute_time()) > b->debounce_time) {
        b->last_state = current_state;
        if (b->last_state == b->pressed_state) {
            return BUTTON_PRESS;
        } else {
            return BUTTON_RELEASE;
        }
    }
    return BUTTON_NONE;
}
