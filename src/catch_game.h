#ifndef CATCH_GAME_H
#define CATCH_GAME_H

#define CG_EASY_LIMIT             7
#define CG_NORMAL_LIMIT           5
#define CG_HARD_LIMIT             4
#define CG_EASY_BASE_DELAY        1500
#define CG_NORMAL_BASE_DELAY      1000
#define CG_HARD_BASE_DELAY        1000
#define CG_EASY_DELAY_REDUCTION   30
#define CG_NORMAL_DELAY_REDUCTION 30
#define CG_HARD_DELAY_REDUCTION   40
// therefore 1 / CG_MINIMUM_DELAY * 1000 is the max number of colors per second
#define CG_MINIMUM_DELAY 333

#include "game_settings.h"
#include "simon_hardware.h"

typedef struct {
    int8_t* seq;
    uint size;
    volatile uint usr_pos;
    volatile uint cpu_pos;
    uint32_t delay;
    uint32_t delay_reduction;
    game_settings_t* settings;
    simon_hardware_t* shw;
} catch_game_t;

bool cg_init(catch_game_t* game, game_settings_t* settings,
             simon_hardware_t* shw);
void cg_deinit(catch_game_t* game);

void cg_start(catch_game_t* game);

#endif /* end of include guard: CATCH_GAME_H */
