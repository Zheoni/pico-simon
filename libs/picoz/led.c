#include "led.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/time.h"

void led_init(uint led_pin, led* l) {
    l->pin = led_pin;
    l->is_pwm_enabled = false;
    l->slice = pwm_gpio_to_slice_num(led_pin);
    l->channel = pwm_gpio_to_channel(led_pin);

    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
}

void led_enable_pwm(led* l) {
    if (l->is_pwm_enabled) return;

    bool current_state = gpio_get(l->pin);
    uint16_t cc = current_state ? 0xFFFE : 0x0000;

    gpio_set_function(l->pin, GPIO_FUNC_PWM);
    pwm_config config = pwm_get_default_config();
    pwm_init(l->slice, &config, true);
    pwm_set_chan_level(l->slice, l->channel, cc);

    l->is_pwm_enabled = true;
}

void led_disable_pwm(led* l) {
    if (!l->is_pwm_enabled) return;

    uint16_t cc = pwm_hw->slice[l->slice].cc;
    cc = (cc >> (l->channel ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB)) & 0xFFFF;
    bool state = cc > 0;

    uint cc_other = pwm_hw->slice[l->slice].cc;
    cc_other = (cc_other >> (l->channel ? PWM_CH0_CC_A_LSB : PWM_CH0_CC_B_LSB)) & 0xFFFF;
    if (cc_other == 0) {
        pwm_set_enabled(l->slice, false);
    }

    gpio_set_function(l->pin, GPIO_FUNC_SIO);
    gpio_put(l->pin, state);

    l->is_pwm_enabled = false;
}

void led_put(led* l, bool value) {
    if (l->is_pwm_enabled) return;
    gpio_put(l->pin, value);
}

void led_toggle(led* l) {
    if (l->is_pwm_enabled) {
        uint16_t cc = pwm_hw->slice[l->slice].cc;
        cc = (cc >> (l->channel ? PWM_CH0_CC_B_LSB : PWM_CH0_CC_A_LSB)) & 0xFFFF;
        pwm_set_chan_level(l->slice, l->channel, !cc);
    } else {
        gpio_put(l->pin, !gpio_get(l->pin));
    }
}

void led_on(led* l) {
    if (l->is_pwm_enabled) {
        pwm_set_chan_level(l->slice, l->channel, 255 * 255);
    } else {
        gpio_put(l->pin, 1);
    }
}

void led_off(led* l) {
    if (l->is_pwm_enabled) {
        pwm_set_chan_level(l->slice, l->channel, 0);
    } else {
        gpio_put(l->pin, 0);
    }
}

void led_level(led* l, uint8_t level) {
    if (!l->is_pwm_enabled) return;
    pwm_set_chan_level(l->slice, l->channel, level * level);
}

static bool _led_pulsating_callback(repeating_timer_t* rt) {
    led* l = (led*) rt->user_data;

    led_level(l, l->intensity);

    if (l->intensity == 255) {
        l->direction = 0;
    } else if (l->intensity == 0) {
        l->direction = 1;
    }

    l->intensity += l->direction * 2 - 1;

    return true;
}

bool led_start_pulsating(led* l, int32_t delay_ms) {
    if (!l->is_pwm_enabled) return false;

    l->intensity = 0;
    l->direction = 1;
    return add_repeating_timer_ms(delay_ms, _led_pulsating_callback, (void*) l, &l->timer);
}

bool led_stop_pulsating(led* l) {
    if (!l->is_pwm_enabled) return false;

    return cancel_repeating_timer(&l->timer);
}
