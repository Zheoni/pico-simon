#include "sequences.h"


// freq , delay
note VICTORY_SEQUENCE[] = {
    { 660   ,   150 },
    { 0     ,   20  },
    { 880   ,   400 },
    BUZZER_END_SEQUENCE
};

note LOOSE_SEQUENCE[] = {
    { 180   ,   300  },
    { 0     ,   20   },
    { 138   ,   300  },
    { 110   ,   1200 },
    BUZZER_END_SEQUENCE
};

note READY_SEQUENCE[] = {
    { 1760  ,   100 },
    BUZZER_END_SEQUENCE
};

void start_sequence(shw_t* shw) {
    for (int i = 0; i < 5; ++i) {
        for (int l = 0; l < N_COLORS; ++l) {
            led_on(&shw->leds[l]);
            sleep_ms(100);
            led_off(&shw->leds[l]);
        }
    }
}

void overflow_sequence(shw_t* shw) {
    led_enable_pwm(&shw->leds[0]);
    led_start_pulsating(&shw->leds[0], 1);
    buzzer_play_sound_sequence(shw->buzzer, VICTORY_SEQUENCE);
    buzzer_play_sound_sequence(shw->buzzer, VICTORY_SEQUENCE);
    buzzer_play_sound_sequence(shw->buzzer, VICTORY_SEQUENCE);
    led_stop_pulsating(&shw->leds[0]);
    led_disable_pwm(&shw->leds[0]);
}

void victory_sequence(shw_t* shw, bool sound_enabled, bool leds_enabled) {
    if (sound_enabled)
        buzzer_play_sound_sequence(shw->buzzer, VICTORY_SEQUENCE);
}

void loose_sequence(shw_t* shw, bool sound_enabled, bool leds_enabled) {
    if (sound_enabled)
        buzzer_play_sound_sequence_non_blocking(shw->buzzer, LOOSE_SEQUENCE);
    if (leds_enabled) {
        for (int k = 0; k < 12; ++k) {
            for (int i = 0; i < N_COLORS; ++i)
                led_put(&shw->leds[i], k % 2 == 0);
            sleep_ms(50);
        }
    }
    buzzer_block_until_sequences_finish();
}

void ready_sequence(shw_t* shw, bool sound_enabled, bool leds_enabled) {
    if (sound_enabled)
        buzzer_play_sound_sequence_non_blocking(shw->buzzer, READY_SEQUENCE);
    if (leds_enabled) {
        for (int i = 0; i < N_COLORS; ++i) led_on(&shw->leds[i]);
        sleep_ms(75);
        for (int i = 0; i < N_COLORS; ++i) led_off(&shw->leds[i]);
    }
    buzzer_block_until_sequences_finish();
}
