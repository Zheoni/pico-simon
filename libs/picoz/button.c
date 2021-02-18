#include "button.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "button_debounce.pio.h"

const static PIO pios[2] = { pio0, pio1 };
static uint sm_count[2] = { 0 };
static uint offset[2];

bool button_init(uint button_pin, enum button_pull pull, bool pressed_state,
                 button* b) {
    b->pin = button_pin;
    gpio_init(button_pin);
    gpio_set_dir(button_pin, GPIO_IN);
    gpio_set_pulls(button_pin, pull == BUTTON_PULL_UP,
                   pull == BUTTON_PULL_DOWN);

    b->pressed_state = pressed_state;
    b->last_state = !pressed_state;

    // get an unused state machine from either PIOs
    b->sm = pio_claim_unused_sm(pio0, false);
    if (b->sm != -1) {
        b->pio_idx = 0;
    } else {
        b->sm = pio_claim_unused_sm(pio1, false);
        if (b->sm != -1) {
            b->pio_idx = 1;
        } else {
            return false;
        }
    }

    // load the program into the PIO if its not loaded
    if (sm_count[b->pio_idx] == 0) {
        if (pio_can_add_program(pios[b->pio_idx], &button_debounce_program)) {
            offset[b->pio_idx] =
                pio_add_program(pios[b->pio_idx], &button_debounce_program);
            ++sm_count[b->pio_idx];
        } else {
            return false;
        }
    }

    // Configure the state machine
    pio_sm_config c =
        button_debounce_program_get_default_config(offset[b->pio_idx]);
    sm_config_set_in_pins(&c, button_pin);
    sm_config_set_jmp_pin(&c, button_pin);
    sm_config_set_clkdiv_int_frac(&c, 40322, 149); // ~ 20 ms

    // Apply the config
    pio_sm_init(pios[b->pio_idx], b->sm, offset[b->pio_idx], &c);
    pio_sm_set_enabled(pios[b->pio_idx], b->sm, true);

    return true;
}

void button_deinit(button* b) {
    pio_sm_set_enabled(pios[b->pio_idx], b->sm, false);
    pio_sm_unclaim(pios[b->pio_idx], b->sm);
    --sm_count[b->pio_idx];
    if (sm_count[b->pio_idx] == 0) {
        pio_remove_program(pios[b->pio_idx], &button_debounce_program,
                           offset[b->pio_idx]);
    }
}

bool button_get(button* b) {
    uint pc = pio_sm_get_pc(pios[b->pio_idx], b->sm);
    return pc >= offset[b->pio_idx] + 6;
}

bool button_set_debounce_time_us(button* b, uint32_t debounce_time_us) {
    uint32_t sysclock = clock_get_hz(clk_sys);

    float div = sysclock * debounce_time_us / 62000000.0;
    if (div > 0xffff)
        div = 0xffff;
    pio_sm_set_clkdiv(pios[b->pio_idx], b->sm, div);
}

void button_prepare_for_loop(button* b) {
    b->last_state = button_get(b);
}

button_change_t button_change_steady(button* b) {
    bool current_state = button_get(b);
    if (current_state != b->last_state) {
        b->last_state = current_state;
        if (current_state & b->pressed_state) {
            return BUTTON_PRESS;
        } else {
            return BUTTON_RELEASE;
        }
    }
    return BUTTON_NONE;
}
