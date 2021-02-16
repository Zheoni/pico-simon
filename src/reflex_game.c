#include "reflex_game.h"
#include "sequences.h"

#include "hardware/gpio.h"
#include "pico/time.h"
#include <stdlib.h>

struct game_data {
    shw_t* shw;
    game_settings_t* settings;
    alarm_id_t taunt_alarm;
};

static int64_t _trigger(alarm_id_t id, void* user_data) {
    struct game_data* data = (struct game_data*) user_data;

    cancel_alarm(data->taunt_alarm);

    if (data->settings->leds_enabled) {
        for (int i = 0; i < N_COLORS; ++i) {
            led_on(&data->shw->leds[i]);
        }
    }

    if (data->settings->sound_enabled) {
        buzzer_play_sound_sequence_non_blocking(data->shw->buzzer, READY_SEQUENCE);
    }

    return 0;
}

static int64_t _taunts(alarm_id_t id, void* user_data) {
    struct game_data* data = (struct game_data*) user_data;

    // 40% normal; 60% hard
    if ((rand() % 100) < (20 + 20 * data->settings->difficulty)) {
        uint l = rand() % N_COLORS;
        led_level(&data->shw->leds[l], rand() % 64 + rand() % 32 + 16);
        busy_wait_us(rand() % 90000 + 10000);
        led_level(&data->shw->leds[l], 64);
    }

    return rand() % 500000 + 750000;
}

void rg_start(game_settings_t* settings, shw_t* shw) {
    uint32_t button_pins = 0;
    for (int i = 0; i < N_COLORS; ++i) {
        led_enable_pwm(&shw->leds[i]);
        button_pins |= 1u << shw->buttons[i].pin;
    }

    prepare_sequence(shw, settings->sound_enabled, settings->leds_enabled);

    uint32_t min = RG_MIN_TRIGGER_DELAY;
    uint32_t max;
    switch (settings->difficulty) {
        case EASY: max = RG_MAX_EASY_TRIGGER_DELAY;
        break;
        default:
        case NORMAL: max = RG_MAX_NORMAL_TRIGGER_DELAY;
        break;
        case HARD: max = RG_MAX_HARD_TRIGGER_DELAY;
        break;
    }
    uint32_t delay_ms = rand() % (max - min) + min;
    uint led = rand() % N_COLORS;

    struct game_data data = { shw, settings, -1 };

    alarm_id_t trigger_alarm = add_alarm_in_ms(delay_ms, _trigger, &data, true);
    if (settings->leds_enabled && settings->difficulty > EASY) {
        data.taunt_alarm = add_alarm_in_ms(1000, _taunts, &data, true);
    }
    // timeout to not be blocked in the loop forever
    absolute_time_t timeout = make_timeout_time_ms(5000 + delay_ms);
    absolute_time_t start_time = get_absolute_time();

    uint32_t pressed_button_pins;
    do {
        // get all pins at the same time as fast as possible, no debounce
        // as we only one the first press
        pressed_button_pins = button_pins & gpio_get_all();
    } while (pressed_button_pins == 0 && !time_reached(timeout));

    // cancel the alarms because they may have not been fired
    cancel_alarm(trigger_alarm);
    cancel_alarm(data.taunt_alarm);

    absolute_time_t press_time = get_absolute_time();

    for (int i = 0; i < N_COLORS; ++i) {
        led_off(&shw->leds[i]);
    }

    // if the press was before the alarm or there was no press
    if (absolute_time_diff_us(start_time, press_time) < delay_ms * 1000 || pressed_button_pins == 0) {
        for (int i = 0; i < N_COLORS; ++i) {
            led_disable_pwm(&shw->leds[i]);
        }
        loose_sequence(shw, settings->sound_enabled, settings->leds_enabled);
    } else {
        bool winner[N_COLORS] = { false };
        // translate pressed pin numbers to our indices and set the winning LED(s)
        for (int pin = 0; pin < 32; ++pin) {
            // if the pin is an pressed button
            if ((1u << pin) & pressed_button_pins) {
                for (int i = 0; i < N_COLORS; ++i) {
                    // get the button by matching the pin numbers
                    if (shw->buttons[i].pin == pin) {
                        winner[i] = true;
                    }
                }
            }
        }

        // Show the winners
        for (int i = 0; i < N_COLORS; ++i) {
            if (settings->sound_enabled)
                buzzer_play_sound_sequence_non_blocking(shw->buzzer, VICTORY_SEQUENCE);
            if (winner[i]) {
                led_start_pulsating(&shw->leds[i], 1);
            }
        }
        sleep_ms(2000);
        for (int i = 0; i < N_COLORS; ++i) {
            if (winner[i]) {
                led_stop_pulsating(&shw->leds[i]);
            }
            led_disable_pwm(&shw->leds[i]);
            led_off(&shw->leds[i]);
        }
    }
}
