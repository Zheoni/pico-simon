#ifndef LED_H
#define LED_H

#include "pico/types.h"
#include "pico/time.h"

typedef struct {
    uint pin;
    uint slice;
    uint channel;
    uint8_t intensity;
    bool is_pwm_enabled;
    bool direction;
    repeating_timer_t timer;
} led;

void led_init(uint led_pin, led* l);

void led_enable_pwm(led* l);

void led_disable_pwm(led* l);

void led_put(led* l, bool value);

void led_toggle(led* l);

void led_on(led* l);

void led_off(led* l);

void led_level(led* l, uint8_t level);

bool led_start_pulsating(led* l, int32_t delay_ms);

bool led_stop_pulsating(led* l);

#endif /* end of include guard: LED_H */
