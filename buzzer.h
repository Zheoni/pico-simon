#ifndef BUZZER_H
#define BUZZER_H

#include "pico/types.h"

typedef uint32_t sound;

typedef struct {
    sound s;
    uint d;
} note;

// Calc a sound from a frequency.
// When return 0, ERROR.
sound buzzer_calc_sound(uint freq);

void buzzer_init(uint buzzer_pin);

void buzzer_play_sound(uint buzzer_pin, sound sound);
void buzzer_stop_sound(uint buzzer_pin);

void buzzer_play_sound_sequence(uint buzzer_pin, note* notes, uint n);

#endif /* end of include guard: BUZZER_H */
