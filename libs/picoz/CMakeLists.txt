cmake_minimum_required(VERSION 3.13)

add_library(picoz
    button.c
    buzzer.c
    led.c
    util.c
)

pico_generate_pio_header(picoz ${CMAKE_CURRENT_LIST_DIR}/button_debounce.pio)

target_include_directories(picoz PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(picoz
    pico_stdlib
    hardware_pwm
    hardware_pio
)
