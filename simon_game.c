#include "simon_game.h"

#include "sequences.h"
#include <stdlib.h>

extern sound COLOR_SOUNDS[];

// TODO: Review this function behaviour
static uint32_t calc_linear_delay(uint pos) {
    uint32_t reduction = pos * SG_DELAY_REDUCTION_PER_ROUND;
    if (reduction > SG_MAX_DELAY - SG_MIN_DELAY) {
        return SG_MIN_DELAY;
    }
    return SG_MAX_DELAY - reduction;
}

static void sg_display_seq(simon_game_t* game) {
    shw_t* shw = game->shw;
    game_settings_t* settings = game->settings;
    uint8_t color;
    uint32_t delay;

    for (uint i = 0; i < game->n; ++i) {
        color = game->seq[i];
        if (settings->leds_enabled) led_on(&shw->leds[color]);
        if (settings->sound_enabled) {
            buzzer_play_sound(shw->buzzer, COLOR_SOUNDS[color]);
            sleep_ms(100);
            buzzer_stop_sound(shw->buzzer);
        }
        sleep_ms(200 + 100 * !settings->sound_enabled);
        if (settings->leds_enabled) led_off(&shw->leds[color]);

        switch (settings->difficulty) {
            case EASY: delay = SG_MAX_DELAY;
            break;
            default:
            case NORMAL: delay = calc_linear_delay(game->n - i - 1);
            break;
            case HARD: delay = calc_linear_delay(game->n - i - 1) >> 1;
            break;
        }
        sleep_ms(delay);
    }
}

static bool sg_guess_round(simon_game_t* game) {
    shw_t* shw = game->shw;
    game_settings_t* settings = game->settings;
    uint8_t color;
    bool guessed;

    uint32_t time_limit, time_start;
    switch (settings->difficulty) {
        case EASY: time_limit = 60 * 1000 * 1000; // a minute...
        break;
        default:
        case NORMAL: time_limit = SG_TIME_LIMIT_NORMAL;
        break;
        case HARD: time_limit = SG_TIME_LIMIT_HARD;
        break;
    }
    time_limit *= 1000; // to microseconds
    for (uint i = 0; i < game->n; ++i) {
        color = game->seq[i];
        guessed = false;
        time_start = get_absolute_time();
        while (!guessed) {
            for (int b = 0; b < N_COLORS; ++b) {
                if (button_change_steady(&shw->buttons[b]) == BUTTON_PRESS) {
                    if (b == color) {
                        if (settings->sound_enabled) buzzer_play_sound(shw->buzzer, COLOR_SOUNDS[color]);
                        if (settings->leds_enabled) led_on(&shw->leds[b]);
                        while (button_change_steady(&shw->buttons[b]) != BUTTON_RELEASE);
                        if (settings->sound_enabled) buzzer_stop_sound(shw->buzzer);
                        if (settings->leds_enabled) led_off(&shw->leds[b]);
                        guessed = true;
                        break;
                    } else {
                        return false;
                    }
                }
            }
            // timeout by time_limit, even if the user is correct but kept the
            // button pressed
            if (absolute_time_diff_us(time_start, get_absolute_time()) > time_limit) {
                return false;
            }
        }
    }
    return true;
}

/**
* Adds a color to the sequence. Returns false when cannot allocate
*/
static bool sg_next_round(simon_game_t* game) {
    if (game->n >= game->size) {
        // allocate more space
        game->size = game->size << 1; // duplicate the space
        game->seq = (uint8_t*) realloc(game->seq, game->size * sizeof(uint8_t));
        if (game->seq == NULL) return false;
    }
    uint8_t next_color = rand() % N_COLORS;
    game->seq[game->n++] = next_color;
    return true;
}

/**
* Initializes the game. Returns false when cannot allocate
*/
bool sg_init(simon_game_t* game, game_settings_t* settings, shw_t* shw) {
    game->n = 0;
    game->seq = (uint8_t*) malloc(ALLOC_BASE_SIZE * sizeof(uint8_t));
    if (game->seq == NULL) return false;
    game->size = ALLOC_BASE_SIZE;
    game->settings = settings;
    game->shw = shw;
    return true;
}

bool sg_deinit(simon_game_t* game) {
    free(game->seq);
}

void sg_start(simon_game_t* game) {
    shw_t* shw = game->shw;
    game_settings_t* settings = game->settings;
    bool correct;
    while (1) {
        if (!sg_next_round(game)) {
            // cannot allocate more colors
            overflow_sequence(shw);
            break;
        }
        sg_display_seq(game);
        ready_sequence(shw, settings->sound_enabled, settings->leds_enabled);
        correct = sg_guess_round(game);
        if (correct) {
            sleep_ms(300);
            victory_sequence(shw, settings->sound_enabled, settings->leds_enabled);
            sleep_ms(1500);
        } else {
            loose_sequence(shw, settings->sound_enabled, settings->leds_enabled);
            break;
        }
    }
}
