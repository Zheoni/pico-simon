#ifndef SIMON_HARDWARE_H
#define SIMON_HARDWARE_H

#include "led.h"
#include "button.h"
#include "buzzer.h"
#include "pico/types.h"

// The number of colors (buttons and leds)
#define N_COLORS 4

typedef struct {
    led leds[N_COLORS];
    button buttons[N_COLORS];
    uint buzzer;
} simon_hardware_t;

typedef simon_hardware_t shw_t;

#endif /* end of include guard: SIMON_HARDWARE_H */