#ifndef SIMON_GAME_H
#define SIMON_GAME_H

#include "game_settings.h"
#include "simon_hardware.h"

#define SG_MAX_DELAY 800
#define SG_MIN_DELAY 300
#define SG_DELAY_REDUCTION_PER_ROUND 40
#define SG_TIME_LIMIT_EASY 60000
#define SG_TIME_LIMIT_NORMAL 10000
#define SG_TIME_LIMIT_HARD 4000

#define ALLOC_BASE_SIZE 16

typedef struct {
    uint8_t* seq;
    uint size;
    uint n;
    game_settings_t* settings;
    simon_hardware_t* shw;
} simon_game_t;

bool sg_init(simon_game_t* game, game_settings_t* settings, shw_t* shw);
void sg_deinit(simon_game_t* game);

void sg_start(simon_game_t* game);

#endif /* end of include guard: SIMON_GAME_H */
