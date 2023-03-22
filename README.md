# Frogger

Basic implementation of frogger for Raspberry Pi Pico (RP2040) with a monochrome OLED display (SSD1306) connected using SPI.

# Hardware
This project is built mainly for [Pico Game Board](https://github.com/danielkurek/PiPico_game_board). If you want to change pin numbers, you can do at the beginning of `frogger.cpp`.

## Button pins

| Buttons          | Pin |
|------------------|-----|
| Up (SW_UP1)      | GP0 |
| Down (SW_DWN1)   | GP1 |
| Left (SW_L1)     | GP2 |
| Right (SW_R1)    | GP3 |
| Back (SW_BCK1)   | GP4 |
| Action (SW_ACT1) | GP5 |

## Display pins

| Display pins | Pico Pins   |
|--------------|-------------|
| GND          | GND         |
| VDD          | 3V3         |
| SCK          | GP10 (SPI1) |
| SDA          | GP11 (SPI1) |
| RES          | GP7         |
| DC           | GP8         |
| CS           | GP9         |

# Code overview
 - libraries
   - Game engine (`game_engine`)
   - Button debouncing (`button`)
   - OLED display driver (`pico_ssd1306`)
 - main file `frogger.cpp`

## OLED display driver
The [pico_ssd1306](https://github.com/danielkurek/pico_ssd1306) driver implements communication with SSD1306 displays with some basic graphic functions. 
It is a fork of [daschr/pico-ssd1306](https://github.com/daschr/pico-ssd1306) which is modified to use SPI for communication with the display

## Button debouncing
Simple `Button` class located in `button` library is responsible for debouncing of buttons.

The contructor initializes the gpio pin and configures the pin to internal __PULL DOWN__.

The method `isPressed` checks the state of the button and returns `true` only after the button is pressed for at least `debounce_time` microseconds (without fluctuating). It returns `true` only once for pressed button (even if the button is held for a long time). 

I am using 10000 us (10 ms) as __debounce time__ and it works great. You can try increasing it if you are experiencing double clicks. The value depends a lot on the buttons used and how worn they are.


## Game engine
 - `GameEngine` class is responsible for managing the game loop and checking if the game should end or not
 - `GameObject` class is a base class for any object that should be displayed
   - does not move
 - `PhysicsObject` class is a child of `GameObject`
   - has a motion vector and step time which indicates that every step it should move by the motion vector
   - it can detect collisions with other objects
   - updates its position everytime `updateTick` is called
 - `Frog` class is a child of `PhysicsObject`
   - static members for its image
   - it moves by pressing buttons

## Main file
 - initializes GPIO and display
 - starts the game with a button press (also for restarts after game ends)
 - spawns game objects into game engine