// Header guard: stops this file from being included twice by accident,
// which would cause "already defined" errors when compiling.
#ifndef _FREENOVE_4WD_CAR_WS2812_H
#define _FREENOVE_4WD_CAR_WS2812_H

#include <Adafruit_NeoPixel.h>   // Library from Adafruit that handles the tricky timing needed to talk to WS2812 LEDs

#define WS2812_PIN 16        // The Pico W pin wired to the LED strip's data line
#define LEDS_COUNT 8         // How many individual RGB LEDs are on the strip
extern int ws2812_task_mode; // Remembers which lighting mode (see WS2812_Show) is currently running

// --- Function declarations (the "menu" of what this LED module can do) ---
// The actual code for each of these lives in Freenove_4WD_Car_WS2812.cpp

void WS2812_Setup(void);                                                                                   // Call once at startup to connect to the LED strip
void WS2812_Show(int mode);                                                                                // Call every loop() to keep the chosen light pattern animating
void WS2812_Set_Color_1(int number, unsigned char color_1,unsigned char color_2,unsigned char color_3);    // Choose "color slot 1" (red, green, blue each 0-255) for chosen LEDs
void WS2812_Set_Color_2(int number, unsigned char color_1,unsigned char color_2,unsigned char color_3);    // Choose "color slot 2" (used e.g. for the second color in blink mode)
void WS2812_SetMode(int mode);                                                                             // Change which lighting mode is active (0-5)
void ws2812_close(void);      // Turn all LEDs off (black / 0,0,0)
void ws2812_following(void);  // "Chasing lights" pattern: LEDs light up one after another
void ws2812_rgb(void);        // Show a plain, unmoving color on all LEDs
void ws2812_rgb(void);
void ws2812_breathe(void);    // Smoothly fade brightness up and down, like slow breathing
void ws2812_rainbow(void);    // Cycle a scrolling rainbow of colors across the strip
int Wheel(byte pos);          // Helper: turns a position (0-255) on a color wheel into an RGB color

#endif













//
