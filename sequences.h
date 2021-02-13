#ifndef SEQUENCES_H
#define SEQUENCES_H

#include "simon_hardware.h"

extern note VICTORY_SEQUENCE[];
extern note LOOSE_SEQUENCE[];
extern note READY_SEQUENCE[];

void start_sequence(shw_t* shw);
void overflow_sequence(shw_t* shw);
void victory_sequence(shw_t* shw, bool sound_enabled, bool leds_enabled);
void loose_sequence(shw_t* shw, bool sound_enabled, bool leds_enabled);
void ready_sequence(shw_t* shw, bool sound_enabled, bool leds_enabled);

#endif /* end of include guard: SEQUENCES_H */
