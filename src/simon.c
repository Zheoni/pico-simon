#include "hardware/regs/rosc.h"
#include "pico/divider.h"
#include "pico/time.h"
#include <stdlib.h>

#include "picoz/button.h"
#include "picoz/buzzer.h"
#include "picoz/led.h"
#include "picoz/util.h"

#include "game_settings.h"
#include "sequences.h"
#include "simon_hardware.h"

#include "catch_game.h"
#include "reflex_game.h"
#include "simon_game.h"

// The number of LEDs and BUTTONs must be the same!
// Order is RED, BLUE, YELLOW, GREEN
static const uint LED_PINS[N_COLORS] = { 13, 12, 11, 10 };
static const uint BUTTON_PINS[N_COLORS] = { 18, 19, 20, 21 };
static const uint BUZZER_PIN = 9;
// Make sure that the buzzer pin does not share the pwm
// slice with any of the led pins.
// See rp2040 datasheet 4.5.2

// the frequencies will change to other calculated values, don't use them
// directly expecting them to be the same
sound COLOR_SOUNDS[] = {
    440, // La / A / Red
    330, // Mi / E / Blue
    277, // Do# / C# / Yellow
    165, // Mi / E / Green (octave lower)
};

// END OF CONFIGURATION

static bool setup(simon_hardware_t* shw) {
    bool valid = true;
    for (int i = 0; i < N_COLORS; ++i) {
        // Init the LED pins and set them to be OUT pins
        led_init(LED_PINS[i], &shw->leds[i]);

        // Init the BUTTON pins, set them to be pulled down
        valid &= button_init(BUTTON_PINS[i], BUTTON_PULL_DOWN, true,
                             &shw->buttons[i]);
    }

    // Init the buzzer
    buzzer_init(BUZZER_PIN);
    shw->buzzer = BUZZER_PIN;

    // Calculate sounds
    uint n_sounds = sizeof COLOR_SOUNDS / sizeof COLOR_SOUNDS[0];
    for (uint i = 0; i < n_sounds; ++i) {
        COLOR_SOUNDS[i] = buzzer_calc_sound(COLOR_SOUNDS[i]);
    }

    buzzer_calc_sound_sequence(VICTORY_SEQUENCE, VICTORY_SEQUENCE);
    buzzer_calc_sound_sequence(LOOSE_SEQUENCE, LOOSE_SEQUENCE);
    buzzer_calc_sound_sequence(READY_SEQUENCE, READY_SEQUENCE);
    buzzer_calc_sound_sequence(PREPARE_SEQUENCE, PREPARE_SEQUENCE);

    // Set a random seed
    uint32_t random = 0;
    volatile uint32_t* rnd_reg =
        (uint32_t*) (ROSC_BASE + ROSC_RANDOMBIT_OFFSET);

    for (int i = 0; i < 32; ++i) {
        random = random << 1;
        random = random + (0x00000001 & (*rnd_reg));
    }
    srand(random);

    return valid;
}

int main() {
    simon_hardware_t shw;

    if (!setup(&shw)) {
        return 1;
    }

    start_sequence(&shw);

    sleep_ms(300);

    game_settings_t settings;
    settings_set_default(&settings);

    uint32_t pressed_button;
    while (1) {
        pressed_button = wait_button_push(shw.leds, shw.buttons, N_COLORS);
        switch (pressed_button) {
        default:
        case 0: {
            simon_game_t game;
            if (!sg_init(&game, &settings, &shw))
                overflow_sequence(&shw);
            sleep_ms(800);
            sg_start(&game);
            sg_deinit(&game);
        } break;
        case 1: {
            catch_game_t game;
            if (!cg_init(&game, &settings, &shw))
                overflow_sequence(&shw);
            sleep_ms(800);
            cg_start(&game);
            cg_deinit(&game);
        } break;
        case 2: {
            // wait for button release before starting this game
            rg_start(&settings, &shw);
        } break;
        case 3: {
            sleep_ms(100);
            settings_adjust(&settings, &shw);
        } break;
        case N_COLORS: {
            // timeout, enter low power mode

            // TODO: low power mode is unstable in SDK, just busy until thats
            // stable
            int b = 0;
            while (1) {
                if (button_get(&shw.buttons[b]))
                    break;
                sleep_ms(5);
                if (++b == N_COLORS)
                    b = 0;
            }
        } break;
        }
    }
}
