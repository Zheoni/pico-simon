#include "reflex_game.h"
#include "sequences.h"

#include "hardware/gpio.h"
#include "pico/time.h"
#include <stdlib.h>

struct game_data {
    shw_t* shw;
    game_settings_t* settings;
};

static uint32_t button_pins;
static volatile int winner_pin;
static volatile absolute_time_t press_time;
static alarm_id_t trigger_alarm;
static alarm_id_t taunt_alarm;
static button* buttons;

static void _button_handler(uint gpio, uint32_t events) {
    // if it is one of our buttons
    if ((1u << gpio) & button_pins) {
        press_time = get_absolute_time();

        // disable all the buttons irq, so only the first fires
        for (int i = 0; i < N_COLORS; ++i) {
            gpio_set_irq_enabled(buttons[i].pin, events, false);
        }

        // cancel the alarms to make sure they dont fire from now on
        cancel_alarm(trigger_alarm);
        cancel_alarm(taunt_alarm);

        winner_pin = gpio;
    }
}

static int64_t _trigger(alarm_id_t id, void* user_data) {
    if (winner_pin != -1)
        return 0;

    struct game_data* data = (struct game_data*) user_data;

    // if taunts are enabled, cancel them
    if (taunt_alarm != -1)
        cancel_alarm(taunt_alarm);

    if (data->settings->leds_enabled) {
        for (int i = 0; i < N_COLORS; ++i) {
            led_on(&data->shw->leds[i]);
        }
    }

    if (data->settings->sound_enabled) {
        buzzer_play_sound_sequence_non_blocking(data->shw->buzzer,
                                                READY_SEQUENCE);
    }

    return 0;
}

static int64_t _taunts(alarm_id_t id, void* user_data) {
    if (winner_pin != -1)
        return 0;

    struct game_data* data = (struct game_data*) user_data;

    // 40% normal; 60% hard
    if ((rand() % 100) < (20 + 20 * data->settings->difficulty)) {
        // select a random led
        uint l = rand() % N_COLORS;
        // set increase or decrease a little bit the level randomly
        led_level(&data->shw->leds[l], rand() % 64 + rand() % 32 + 16);
        // wait between 10 and 100 milliseconds
        busy_wait_us(rand() % 90000 + 10000);
        // restore the led level
        led_level(&data->shw->leds[l], 64);
    }

    // repeat again in between 750 and 1500 milliseconds
    return -(rand() % 500000 + 750000);
}

/**
 * Starts the reflex game.
 * IMPORTANT:
 * ! MAKE SURE TO **NOT** CALL THIS FUNCTION AGAIN UNTIL IT FINISHES!!!
 */
void rg_start(game_settings_t* settings, shw_t* shw) {
    button_pins = 0;
    winner_pin = -1;
    press_time = nil_time;
    trigger_alarm = -1;
    taunt_alarm = -1;
    buttons = shw->buttons;

    for (int i = 0; i < N_COLORS; ++i) {
        led_enable_pwm(&shw->leds[i]);
        button_pins |= 1u << shw->buttons[i].pin;
    }

    prepare_sequence(shw, settings->sound_enabled, settings->leds_enabled);

    uint32_t min = RG_MIN_TRIGGER_DELAY;
    uint32_t max;
    switch (settings->difficulty) {
    case EASY:
        max = RG_MAX_EASY_TRIGGER_DELAY;
        break;
    default:
    case NORMAL:
        max = RG_MAX_NORMAL_TRIGGER_DELAY;
        break;
    case HARD:
        max = RG_MAX_HARD_TRIGGER_DELAY;
        break;
    }
    uint32_t delay_ms = rand() % (max - min) + min;

    struct game_data data = { shw, settings };

    trigger_alarm = add_alarm_in_ms(delay_ms, &_trigger, &data, true);
    if (settings->leds_enabled && settings->difficulty > EASY) {
        taunt_alarm = add_alarm_in_ms(1000, &_taunts, &data, true);
    }
    // timeout to not be blocked in the loop forever
    absolute_time_t timeout = make_timeout_time_ms(5000 + delay_ms);
    absolute_time_t start_time = get_absolute_time();

    for (int i = 0; i < N_COLORS; ++i) {
        gpio_set_irq_enabled_with_callback(
            shw->buttons[i].pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_LEVEL_HIGH, true,
            &_button_handler);
    }

    while (winner_pin == -1 && !time_reached(timeout))
        ;

    if (winner_pin == -1) {
        // if the exit condition was the timeout, cancel the alarms
        cancel_alarm(trigger_alarm);
        cancel_alarm(taunt_alarm);
    }

    for (int i = 0; i < N_COLORS; ++i) {
        led_off(&shw->leds[i]);
    }

    // if the press was before the alarm or there was no press
    if (absolute_time_diff_us(start_time, press_time) < delay_ms * 1000 ||
        winner_pin == -1) {
        for (int i = 0; i < N_COLORS; ++i) {
            led_disable_pwm(&shw->leds[i]);
        }
        loose_sequence(shw, settings->sound_enabled, settings->leds_enabled);
    } else {
        uint winner;
        for (int i = 0; i < N_COLORS; ++i) {
            // get the button by matching the pin numbers
            if (shw->buttons[i].pin == (uint) winner_pin) {
                winner = i;
            }
        }

        // Show the winner
        if (settings->sound_enabled)
            buzzer_play_sound_sequence_non_blocking(shw->buzzer,
                                                    VICTORY_SEQUENCE);
        led_start_pulsating(&shw->leds[winner], 1);
        sleep_ms(2000);
        led_stop_pulsating(&shw->leds[winner]);
        for (int i = 0; i < N_COLORS; ++i) {
            led_disable_pwm(&shw->leds[i]);
            led_off(&shw->leds[i]);
        }
    }
}
