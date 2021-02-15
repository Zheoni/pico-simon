#include "game_settings.h"

#include "picoz/util.h"

void settings_set_default(game_settings_t* s) {
    s->sound_enabled = true;
    s->leds_enabled = true;
    s->difficulty = NORMAL;
}

void settings_adjust(game_settings_t* s, shw_t* shw) {
    bool exit = false;
    uint16_t current_bit;
    uint button;
    const uint32_t timeout_ms = 20000;
    while (!exit) {
        button = wait_button_push_detailed(0b11, 0, 1, shw->leds, shw->buttons, N_COLORS, timeout_ms);
        switch (button) {
            case 0:
            // set difficulty
            current_bit = (1 << s->difficulty);
            button = wait_button_push_detailed(0b111 ^ current_bit, current_bit, 1, shw->leds, shw->buttons, N_COLORS, timeout_ms);
            if (button <= HARD) {
                s->difficulty = button;
            } else if (button == N_COLORS) {
                exit = true;
            }
            break;

            case 1:
            // set sound and leds
            if (s->sound_enabled && s->leds_enabled) {
                current_bit = 0b001;
            } else if (s->leds_enabled) {
                current_bit = 0b010;
            } else {
                current_bit = 0b100;
            }
            button = wait_button_push_detailed(0b111 ^ current_bit, current_bit, 1, shw->leds, shw->buttons, N_COLORS, timeout_ms);
            switch (button) {
                case 0:
                s->sound_enabled = true;
                s->leds_enabled = true;
                break;
                case 1:
                s->leds_enabled = true;
                s->sound_enabled = false;
                break;
                case 2:
                s->leds_enabled = false;
                s->sound_enabled = true;
                break;
                case N_COLORS:
                exit = true;
                break;
            }
            break;

            default:
            case N_COLORS:
            exit = true;
            break;
        }
    }
}
