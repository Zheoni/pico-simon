cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(simon)

pico_sdk_init()

add_subdirectory(libs)
add_subdirectory(src)
