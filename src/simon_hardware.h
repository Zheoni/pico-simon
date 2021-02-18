#ifndef SIMON_HARDWARE_H
#define SIMON_HARDWARE_H

#include "pico/types.h"
#include "picoz/button.h"
#include "picoz/buzzer.h"
#include "picoz/led.h"

// The number of colors (buttons and leds)
#define N_COLORS 4

typedef struct {
    led leds[N_COLORS];
    button buttons[N_COLORS];
    uint buzzer;
} simon_hardware_t;

typedef simon_hardware_t shw_t;

#endif /* end of include guard: SIMON_HARDWARE_H */
