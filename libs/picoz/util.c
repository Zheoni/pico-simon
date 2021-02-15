#include "util.h"
#include "pico/time.h"

uint wait_button_push_detailed(uint16_t on_leds_mask, uint16_t pulsating_leds_mask, uint32_t delay_ms, led* leds, button* buttons, uint n, uint32_t timeout_ms) {
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
        button_prepare_for_loop(&buttons[i]);
    }

    absolute_time_t absolute_timeout = make_timeout_time_ms(timeout_ms);
    bool exit = false;
    uint pressed_button;

    while (!exit) {
        for (uint i = 0; i < n; ++i) {
            if (button_change_steady(&buttons[i]) == BUTTON_PRESS) {
                exit = true;
                pressed_button = i;
                break;
            }
        }
        if (timeout_ms != 0 && time_reached(absolute_timeout)) {
            exit = true;
            pressed_button = n;
        }
        sleep_ms(1);
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
    return wait_button_push_detailed(0, 0xffff, 3, leds, buttons, n, 120000);
}
