#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include "simon_hardware.h"

enum difficulty_t { EASY = 0, NORMAL, HARD };

typedef struct {
    bool sound_enabled;
    bool leds_enabled;
    enum difficulty_t difficulty;
} game_settings_t;

void settings_set_default(game_settings_t* s);
void settings_adjust(game_settings_t* s, simon_hardware_t* shw);

#endif /* end of include guard: GAME_SETTINGS_H */
