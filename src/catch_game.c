#include "catch_game.h"

#include "sequences.h"
#include <stdlib.h>

extern sound COLOR_SOUNDS[];

bool cg_init(catch_game_t* game, game_settings_t* settings, shw_t* shw) {
    switch (settings->difficulty) {
        case EASY:
        game->delay = CG_EASY_BASE_DELAY;
        game->delay_reduction = CG_EASY_DELAY_REDUCTION;
        game->size = CG_EASY_LIMIT;
        break;
        case NORMAL:
        game->delay = CG_NORMAL_BASE_DELAY;
        game->delay_reduction = CG_NORMAL_DELAY_REDUCTION;
        game->size = CG_NORMAL_LIMIT;
        break;
        case HARD:
        game->delay = CG_HARD_BASE_DELAY;
        game->delay_reduction = CG_HARD_DELAY_REDUCTION;
        game->size = CG_HARD_LIMIT;
        break;
    }
    game->delay *= 1000; // to microseconds
    game->delay_reduction *= 1000;
    game->size += 1;
    game->seq = (int8_t*) malloc(game->size * sizeof(int8_t));
    if (game->seq == 0) return false; // cannot alloc
    game->usr_pos = 0;
    game->cpu_pos = game->size - 1;
    game->settings = settings;
    game->shw = shw;
    return true;
}

void cg_deinit(catch_game_t* game) {
    free(game->seq);
}

static void cg_cpu_push_color(catch_game_t* game) {
    shw_t* shw = game->shw;
    int8_t color = rand() % N_COLORS;
    game->cpu_pos = (game->cpu_pos + 1) % game->size;
    game->seq[game->cpu_pos] = color;

    if (game->settings->leds_enabled) led_on(&shw->leds[color]);
    if (game->settings->sound_enabled) buzzer_play_sound(shw->buzzer, COLOR_SOUNDS[color]);
    sleep_ms(75);
    if (game->settings->leds_enabled) led_off(&shw->leds[color]);
    if (game->settings->sound_enabled) buzzer_stop_sound(shw->buzzer);
}

void cg_start(catch_game_t* game) {
    shw_t* shw = game->shw;
    absolute_time_t last_push_tick = get_absolute_time(), current_tick;
    bool error = false;

    cg_cpu_push_color(game);

    while (!error) {
        current_tick = get_absolute_time();
        if (absolute_time_diff_us(last_push_tick, current_tick) >= game->delay) {
            last_push_tick = current_tick;

            cg_cpu_push_color(game);

            // update_delay
            game->delay -= game->delay_reduction;
            if (game->delay < CG_MINIMUM_DELAY * 1000) {
                game->delay = CG_MINIMUM_DELAY * 1000;
            }

            // if queue is full
            if ((game->cpu_pos + 2) % game->size == game->usr_pos) {
                // lost, dont push more
                break;
            }
        }

        // if queue is not empty and the color is consumed: go to next color
        if ((game->cpu_pos + 1) % game->size != game->usr_pos &&
            game->seq[game->usr_pos] < 0) {
            game->usr_pos = (game->usr_pos + 1) % game->size;
        }

        for (int b = 0; b < N_COLORS; ++b) {
            if (button_change_steady(&shw->buttons[b]) == BUTTON_PRESS) {
                if (b == game->seq[game->usr_pos]) {
                    // consume color
                    game->seq[game->usr_pos] = -1;
                } else {
                    // error while guessing
                    error = true;
                    break;
                }
            }
        }
    }
    loose_sequence(shw, game->settings->sound_enabled, game->settings->leds_enabled);
}
