cmake_minimum_required(VERSION 3.13)

include_directories(../libs ./)

add_executable(simon
    simon.c
    simon_game.c
    catch_game.c

    game_settings.c
    sequences.c
)

pico_add_extra_outputs(simon)

target_link_libraries(simon
    pico_stdlib
    picoz
)
