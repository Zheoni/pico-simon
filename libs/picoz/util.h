#ifndef UTIL_H
#define UTIL_H

#include "button.h"
#include "led.h"
#include "pico/types.h"

uint wait_button_push(led* leds, button* buttons, uint n);
uint wait_button_push_detailed(uint16_t on_leds_mask,
                               uint16_t pulsating_leds_mask, uint32_t delay_ms,
                               led* leds, button* buttons, uint n,
                               uint32_t timeout_ms);

#endif /* end of include guard: UTIL_H */
