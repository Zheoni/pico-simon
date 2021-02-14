cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(simon)

pico_sdk_init()

add_executable(simon
    simon.c
    simon_game.c
    catch_game.c

    game_settings.c
    sequences.c
    util.c
    buzzer.c
    led.c
    button.c
)

pico_generate_pio_header(simon ${CMAKE_CURRENT_LIST_DIR}/button_debounce.pio)

pico_add_extra_outputs(simon)

target_link_libraries(simon
    pico_stdlib
    hardware_pwm
    hardware_pio
)
