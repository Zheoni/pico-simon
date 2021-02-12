#include "util.h"

uint wait_button_push_detailed(uint16_t on_leds_mask, uint16_t pulsating_leds_mask, uint32_t delay_ms, led* leds, button* buttons, uint n) {
    uint16_t led_bit;
    for (int i = 0; i < n; ++i) {
        led_bit = (1 << i);
        if (on_leds_mask & led_bit) {
            led_on(&leds[i]);
        } else if (pulsating_leds_mask & led_bit) {
            led_enable_pwm(&leds[i]);
            led_start_pulsating(&leds[i], delay_ms);
        } else {
            led_off(&leds[i]);
        }
    }

    bool pressed = false;
    uint pressed_button;

    while (!pressed) {
        for (uint i = 0; i < n; ++i) {
            if (button_change_steady(&buttons[i]) == BUTTON_PRESS) {
                pressed = true;
                pressed_button = i;
                break;
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        led_bit = (1 << i);
        if (pulsating_leds_mask & led_bit) {
            led_stop_pulsating(&leds[i]);
            led_disable_pwm(&leds[i]);
        }
        led_off(&leds[i]);
    }

    return pressed_button;
}

uint wait_button_push(led* leds, button* buttons, uint n) {
    return wait_button_push_detailed(0, 0xffff, 3, leds, buttons, n);
}
