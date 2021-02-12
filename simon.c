#include <stdlib.h>
#include "hardware/regs/rosc.h"
#include "pico/divider.h"
#include "pico/time.h"

#include "buzzer.h"
#include "led.h"
#include "button.h"
#include "util.h"

// CONFIGURATION
#define SG_MAX_DELAY 800
#define SG_MIN_DELAY 300
#define SG_DELAY_REDUCTION_PER_ROUND 40
#define SG_TIME_LIMIT_NORMAL 10000
#define SG_TIME_LIMIT_HARD 4000

#define CG_EASY_LIMIT 7
#define CG_NORMAL_LIMIT 5
#define CG_HARD_LIMIT 4
#define CG_EASY_BASE_DELAY 1500
#define CG_NORMAL_BASE_DELAY 1000
#define CG_HARD_BASE_DELAY 1000
#define CG_EASY_DELAY_REDUCTION 30
#define CG_NORMAL_DELAY_REDUCTION 30
#define CG_HARD_DELAY_REDUCTION 40
// therefore 1 / CG_MINIMUM_DELAY * 1000 is the max number of colors per second
#define CG_MINIMUM_DELAY 333

#define ALLOC_BASE_SIZE 16

// -- Make sure the number of LEDs and BUTTONs are the same!
// order is RED, BLUE, YELLOW, GREEN
#define N 4
static const uint LED_PINS[N]    = { 13, 12, 11, 10 };
static const uint BUTTON_PINS[N] = { 18, 19, 20, 21 };
static const uint BUZZER_PIN     = 9;
// Make sure that the buzzer pin does not share the pwm
// slice with any of the led pins.
// See rp2040 datasheet 4.5.2


// the frequencies will change to other calculated values, don't use them
// directly expecting them to be the same
static uint32_t COLOR_SOUNDS[] = {
    440, // La / A / Red
    330, // Mi / E / Blue
    277, // Do# / C# / Yellow
    165, // Mi / E / Green (octave lower)
};

// freq , delay
static note VICTORY_SEQUENCE[] = {
    { 660   ,   150 },
    { 0     ,   20  },
    { 880   ,   400 },
    BUZZER_END_SEQUENCE
};

static note LOOSE_SEQUENCE[] = {
    { 180   ,   300  },
    { 0     ,   20   },
    { 138   ,   300  },
    { 110   ,   1200 },
    BUZZER_END_SEQUENCE
};

static note READY_SEQUENCE[] = {
    { 1760  ,   100 },
    BUZZER_END_SEQUENCE
};
// END OF CONFIGURATION

static led LEDS[N];
static button BUTTONS[N];




static void setup() {
    for (int i = 0; i < N; ++i) {
        // Init the LED pins and set them to be OUT pins
        led_init(LED_PINS[i], &LEDS[i]);

        // Init the BUTTON pins, set them to be pulled down
        button_init(BUTTON_PINS[i], BUTTON_PULL_DOWN, true, &BUTTONS[i]);
    }

    // Init the buzzer
    buzzer_init(BUZZER_PIN);

    // Calculate sounds
    uint n = sizeof COLOR_SOUNDS / sizeof COLOR_SOUNDS[0];
    for (uint i = 0; i < n; ++i) {
        COLOR_SOUNDS[i] = buzzer_calc_sound(COLOR_SOUNDS[i]);
    }

    buzzer_calc_sound_sequence(VICTORY_SEQUENCE, VICTORY_SEQUENCE);
    buzzer_calc_sound_sequence(LOOSE_SEQUENCE, LOOSE_SEQUENCE);
    buzzer_calc_sound_sequence(READY_SEQUENCE, READY_SEQUENCE);

    // Set a random seed
    uint32_t random = 0;
    volatile uint32_t* rnd_reg = (uint32_t *) (ROSC_BASE + ROSC_RANDOMBIT_OFFSET);

    for(int i = 0; i < 32; ++i){
        random = random << 1;
        random = random + (0x00000001 & (*rnd_reg));
    }
    srand(random);
}

static void start_sequence() {
    for (int i = 0; i < 5; ++i) {
        for (int l = 0; l < N; ++l) {
            led_on(&LEDS[l]);
            sleep_ms(100);
            led_off(&LEDS[l]);
        }
    }
}

static void overflow_sequence() {
    led_enable_pwm(&LEDS[0]);
    led_start_pulsating(&LEDS[0], 1);
    buzzer_play_sound_sequence(BUZZER_PIN, VICTORY_SEQUENCE);
    buzzer_play_sound_sequence(BUZZER_PIN, VICTORY_SEQUENCE);
    buzzer_play_sound_sequence(BUZZER_PIN, VICTORY_SEQUENCE);
    led_stop_pulsating(&LEDS[0]);
    led_disable_pwm(&LEDS[0]);
}

static void victory_sequence(bool sound, bool leds) {
    if (sound)
        buzzer_play_sound_sequence(BUZZER_PIN, VICTORY_SEQUENCE);
}

static void loose_sequence(bool sound, bool leds) {
    if (sound)
        buzzer_play_sound_sequence_non_blocking(BUZZER_PIN, LOOSE_SEQUENCE);
    if (leds) {
        for (int k = 0; k < 12; ++k) {
            for (int i = 0; i < N; ++i)
                led_put(&LEDS[i], k % 2 == 0);
            sleep_ms(50);
        }
    }
    buzzer_block_until_sequences_finish();
}

static void ready_sequence(bool sound, bool leds) {
    if (sound)
        buzzer_play_sound_sequence_non_blocking(BUZZER_PIN, READY_SEQUENCE);
    if (leds) {
        for (int i = 0; i < N; ++i) led_on(&LEDS[i]);
        sleep_ms(75);
        for (int i = 0; i < N; ++i) led_off(&LEDS[i]);
    }
    buzzer_block_until_sequences_finish();
}





enum difficulty_t {
    EASY=0,
    NORMAL,
    HARD
};

typedef struct  {
    bool sound_enabled;
    bool leds_enabled;
    enum difficulty_t difficulty;
} game_settings_t;

static void settings_set_default(game_settings_t* s) {
    s->sound_enabled = true;
    s->leds_enabled = true;
    s->difficulty = NORMAL;
}

static void settings_adjust(game_settings_t* s) {
    bool exit = false;
    uint16_t current_bit;
    uint button;
    const uint32_t timeout_ms = 20000;
    while (!exit) {
        button = wait_button_push_detailed(0b11, 0, 1, LEDS, BUTTONS, N, timeout_ms);
        switch (button) {
            case 0:
            // set difficulty
            current_bit = (1 << s->difficulty);
            button = wait_button_push_detailed(0b111 ^ current_bit, current_bit, 1, LEDS, BUTTONS, N, timeout_ms);
            if (button <= HARD) {
                s->difficulty = button;
            } else if (button == N) {
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
            button = wait_button_push_detailed(0b111 ^ current_bit, current_bit, 1, LEDS, BUTTONS, N, timeout_ms);
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
                case N:
                exit = true;
                break;
            }
            break;

            default:
            case N:
            exit = true;
            break;
        }
    }
}






typedef struct {
    uint8_t* seq;
    uint size;
    uint n;
    game_settings_t* settings;
} simon_game_t;

static uint32_t calc_linear_delay(uint pos) {
    uint32_t reduction = pos * SG_DELAY_REDUCTION_PER_ROUND;
    if (reduction > SG_MAX_DELAY - SG_MIN_DELAY) {
        return SG_MIN_DELAY;
    }
    return SG_MAX_DELAY - reduction;
}

static void sg_display_seq(simon_game_t* game) {
    uint8_t color;
    uint32_t delay;

    for (uint i = 0; i < game->n; ++i) {
        color = game->seq[i];
        if (game->settings->leds_enabled) led_on(&LEDS[color]);
        if (game->settings->sound_enabled) {
            buzzer_play_sound(BUZZER_PIN, COLOR_SOUNDS[color]);
            sleep_ms(100);
            buzzer_stop_sound(BUZZER_PIN);
        }
        sleep_ms(200 + 100 * !game->settings->sound_enabled);
        if (game->settings->leds_enabled) led_off(&LEDS[color]);

        switch (game->settings->difficulty) {
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
    uint8_t color;
    bool guessed;

    uint32_t time_limit, time_start;
    switch (game->settings->difficulty) {
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
            for (int b = 0; b < N; ++b) {
                if (button_change_steady(&BUTTONS[b]) == BUTTON_PRESS) {
                    if (b == color) {
                        if (game->settings->sound_enabled) buzzer_play_sound(BUZZER_PIN, COLOR_SOUNDS[color]);
                        if (game->settings->leds_enabled) led_on(&LEDS[b]);
                        while (button_change_steady(&BUTTONS[b]) != BUTTON_RELEASE);
                        if (game->settings->sound_enabled) buzzer_stop_sound(BUZZER_PIN);
                        if (game->settings->leds_enabled) led_off(&LEDS[b]);
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
* Initializes the game. Returns false when cannot allocate
*/
static bool sg_init(simon_game_t* game, game_settings_t* settings) {
    game->n = 0;
    game->seq = (uint8_t*) malloc(ALLOC_BASE_SIZE * sizeof(uint8_t));
    if (game->seq == NULL) return false;
    game->size = ALLOC_BASE_SIZE;
    game->settings = settings;
    return true;
}

static bool sg_deinit(simon_game_t* game) {
    free(game->seq);
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
    uint8_t next_color = rand() % N;
    game->seq[game->n++] = next_color;
    return true;
}

static void sg_start(simon_game_t* game) {
    bool correct;
    while (1) {
        if (!sg_next_round(game)) {
            // cannot allocate more colors
            overflow_sequence();
            break;
        }
        sg_display_seq(game);
        ready_sequence(game->settings->sound_enabled, game->settings->leds_enabled);
        correct = sg_guess_round(game);
        sleep_ms(300);
        if (correct) {
            victory_sequence(game->settings->sound_enabled, game->settings->leds_enabled);
            sleep_ms(1500);
        } else {
            loose_sequence(game->settings->sound_enabled, game->settings->leds_enabled);
            break;
        }
    }
    sg_deinit(game);
}




typedef struct {
    int8_t* seq;
    uint size;
    volatile uint usr_pos;
    volatile uint cpu_pos;
    uint32_t delay;
    uint32_t delay_reduction;
    game_settings_t* settings;
} catch_game_t;

static bool cg_init(catch_game_t* game, game_settings_t* settings) {
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
    return true;
}

static void cg_deinit(catch_game_t* game) {
    free(game->seq);
}

static void cg_cpu_push_color(catch_game_t* game) {
    int8_t color = rand() % N;
    game->cpu_pos = (game->cpu_pos + 1) % game->size;
    game->seq[game->cpu_pos] = color;

    if (game->settings->leds_enabled) led_on(&LEDS[color]);
    if (game->settings->sound_enabled) buzzer_play_sound(BUZZER_PIN, COLOR_SOUNDS[color]);
    sleep_ms(75);
    if (game->settings->leds_enabled) led_off(&LEDS[color]);
    if (game->settings->sound_enabled) buzzer_stop_sound(BUZZER_PIN);
}

static void cg_start(catch_game_t* game) {
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

        for (int b = 0; b < N; ++b) {
            if (button_change_steady(&BUTTONS[b]) == BUTTON_PRESS) {
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
    loose_sequence(game->settings->sound_enabled, game->settings->leds_enabled);
    cg_deinit(game);
}





int main() {
    setup();

    start_sequence();

    sleep_ms(300);

    game_settings_t settings;
    settings_set_default(&settings);

    uint32_t pressed_button;
    while (1) {
        pressed_button = wait_button_push(LEDS, BUTTONS, N);
        if (pressed_button == N) {
            // timeout, enter low power mode

            // TODO: low power mode is unstable in SDK, just busy until thats
            // stable
            int b = 0;
            while (1) {
                if (button_get(&BUTTONS[b])) break;
                sleep_ms(5);
                if (++b == N) b = 0;
            }
            continue;
        }
        sleep_ms(100);
        switch (pressed_button) {
            default:
            case 0: {
                simon_game_t game;
                if (!sg_init(&game, &settings)) overflow_sequence();
                sleep_ms(800);
                sg_start(&game);
            }
            break;
            case 1: {
                catch_game_t game;
                if (!cg_init(&game, &settings)) overflow_sequence();
                sleep_ms(800);
                cg_start(&game);
            }
            break;
            case 2: {
                // other game
            }
            break;
            case 3: {
                settings_adjust(&settings);
            }
            break;
        }
    }
    overflow_sequence();
    led_on(&LEDS[0]);
}
