#ifndef BUZZER_H
#define BUZZER_H

#include "pico/types.h"

typedef uint32_t sound;

typedef struct {
    sound s;
    uint32_t d;
} note;

#define BUZZER_END_SEQUENCE { 0, 0 }

// Calc a sound from a frequency.
// When return 0, ERROR.
sound buzzer_calc_sound(uint freq);

void buzzer_init(uint buzzer_pin);

void buzzer_play_sound(uint buzzer_pin, sound sound);
void buzzer_stop_sound(uint buzzer_pin);

void buzzer_calc_sound_sequence(note* notes_src, note* notes_dest);

void buzzer_play_sound_sequence(uint buzzer_pin, note* notes);
bool buzzer_play_sound_sequence_non_blocking(uint buzzer_pin, note* notes);
void buzzer_block_until_sequences_finish();


#endif /* end of include guard: BUZZER_H */
