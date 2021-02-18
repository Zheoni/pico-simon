# Pico Simon

A [Simon Game](https://en.wikipedia.org/wiki/Simon_(game)) implementation
for the [Raspberry Pi Pico](https://www.raspberrypi.org/products/raspberry-pi-pico/).

## Features

- Includes 3 games:
  - The classic simon game.
  - *Catch me!*, where lights start blinking and the player need to follow
    them without falling behind (with a margin).
  - *Quick draw*, multiplayer, when the lights turn on, the first one to hit
    the button wins.
- Settings for the games where you can choose:
  - from 3 leves of difficulty,
  - playing only with lights, only with sound or with both.

## How does the menu works

- Red button: Simon
- Blue button: *Catch me!*
- Yellow button: *Quick draw*
- Green button: Settings
  - Red: Difficulty
    - Red: easy
    - Blue: medium
    - Yellow: hard
    - Green: back
  - Blue: Sound/Leds
    - Red: both
    - Blue: only Leds
    - Yellow: only sound
    - Green: back
  - Yellow/Green: back

## To compile

There is a `CMakeLists.txt` file to build the project. You should also have [set up the pico C SDK](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) and CMake installed.

```sh
mkdir build
cd build
cmake ..
make
```

## Circuit

![Image of the breadboard](breadboard.png)

I use 220 Ohm resistors, but you can switch those according to your LEDs.

The piezo speaker goes to a different ground pin because if I used the same, it
caught some noise when using PWM to control the LEDs.

The buttons are pulled down by the pico.

This project is licensed under the terms of the MIT license.

## Links

- `pioasm` code for the button debounce. This project had a great idea on how to do this. [https://github.com/GitJer/Button-debouncer](https://github.com/GitJer/Button-debouncer)
- The Pico SDK [https://github.com/raspberrypi/pico-sdk](https://github.com/raspberrypi/pico-sdk)
- Micropython implementation for the pico. I used this to understand how the SDK works in a real environment. [https://github.com/raspberrypi/micropython](https://github.com/raspberrypi/micropython)
