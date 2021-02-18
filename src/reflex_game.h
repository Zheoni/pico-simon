#ifndef REFLEX_GAME_H
#define REFLEX_GAME_H

#include "game_settings.h"
#include "simon_hardware.h"

#define RG_MIN_TRIGGER_DELAY        1000
#define RG_MAX_EASY_TRIGGER_DELAY   5000
#define RG_MAX_NORMAL_TRIGGER_DELAY 7000
#define RG_MAX_HARD_TRIGGER_DELAY   10000

/**
 * Starts the reflex game.
 * IMPORTANT:
 * ! MAKE SURE TO **NOT** CALL THIS FUNCTION AGAIN UNTIL IT FINISHES!!!
 */
void rg_start(game_settings_t* settings, simon_hardware_t* shw);

#endif /* end of include guard: REFLEX_GAME_H */
