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

pico_add_extra_outputs(simon)

pico_enable_stdio_usb(simon 1)

target_link_libraries(simon
    pico_stdlib
    hardware_pwm
)
